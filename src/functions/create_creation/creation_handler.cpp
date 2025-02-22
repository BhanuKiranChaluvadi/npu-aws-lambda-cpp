#include "creation_handler.hpp"
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/platform/Environment.h>

CreationHandler::CreationHandler(
    const DynamoDBService &dynamo_service,
    const S3Service &s3_service,
    const std::string &bucket_name)
    : dynamo_service_(dynamo_service), s3_service_(s3_service), bucket_name_(bucket_name)
{
}

// static const char* TAG = "CreateCreation";

aws::lambda_runtime::invocation_response
CreationHandler::handle_request(Creation &creation)
{
    AWS_LOGSTREAM_INFO("CreateCreation", "Processing creation request");

    try
    {
        // Validate creation object
        if (!creation.validate())
        {
            return aws::lambda_runtime::invocation_response::failure(
                "Invalid creation data", "ValidationError");
        }

        // Generate unique ID and timestamp
        creation.generate_id();

        // Upload image and create thumbnail
        S3Service::UploadResult upload_result;
        try
        {
            upload_result = s3_service_.upload_creation_image(
                creation.creation_id,
                creation.image_data);

            creation.image_key = upload_result.image_key;
            creation.thumbnail_key = upload_result.thumbnail_key;
        }
        catch (const std::exception &e)
        {
            AWS_LOGSTREAM_ERROR("CreateCreation",
                                "Image upload failed: " << e.what());
            return aws::lambda_runtime::invocation_response::failure(
                "Failed to upload image: " + std::string(e.what()),
                "UploadError");
        }

        // Save to DynamoDB
        try
        {
            if (!dynamo_service_.save_creation(creation))
            {
                // Cleanup S3 on DynamoDB failure
                try
                {
                    s3_service_.delete_images(
                        creation.image_key,
                        creation.thumbnail_key);
                }
                catch (const std::exception &e)
                {
                    AWS_LOGSTREAM_WARN("CreateCreation",
                                       "Failed to cleanup S3 after DynamoDB error: " << e.what());
                }

                return aws::lambda_runtime::invocation_response::failure(
                    "Failed to save creation", "DatabaseError");
            }
        }
        catch (const std::exception &e)
        {
            // Cleanup S3 on DynamoDB exception
            try
            {
                s3_service_.delete_images(
                    creation.image_key,
                    creation.thumbnail_key);
            }
            catch (...)
            {
                AWS_LOGSTREAM_WARN("CreateCreation",
                                   "Failed to cleanup S3 after DynamoDB exception");
            }

            return aws::lambda_runtime::invocation_response::failure(
                "Database error: " + std::string(e.what()),
                "DatabaseError");
        }

        // Create success response
        AWS_LOGSTREAM_INFO("CreateCreation",
                           "Successfully created creation with ID: " << creation.creation_id);

        return aws::lambda_runtime::invocation_response::success(
            create_response(creation).View().WriteCompact(),
            "application/json");
    }
    catch (const std::exception &e)
    {
        AWS_LOGSTREAM_ERROR("CreateCreation",
                            "Unhandled exception: " << e.what());
        return aws::lambda_runtime::invocation_response::failure(
            e.what(), "InternalError");
    }
}

Creation CreationHandler::parse_request(const Aws::String& request_payload)
{
    using namespace Aws::Utils::Json;

    Creation creation;

    // Print the raw JSON payload
    AWS_LOGSTREAM_INFO("CreateCreation", "Raw JSON payload: " << request_payload);

    // Parse the entire request
    JsonValue json(request_payload);
    if (!json.WasParseSuccessful())
    {
        throw std::runtime_error("Failed to parse input JSON");
    }

    // Extract the body
    JsonView view = json.View();
    Aws::String body;
    if (view.KeyExists("body"))
    {
        body = view.GetString("body");
    }
    else
    {
        throw std::runtime_error("Missing 'body' in request");
    }

    AWS_LOGSTREAM_INFO("CreateCreation", "Extracted body: " << body);

    // Parse the body
    JsonValue body_json(body);
    if (!body_json.WasParseSuccessful())
    {
        throw std::runtime_error("Failed to parse body JSON");
    }

    JsonView json_data = body_json.View();

    if (!json_data.KeyExists("element_name") ||
        !json_data.KeyExists("title") ||
        !json_data.KeyExists("image_data") ||
        !json_data.KeyExists("user_id"))
    {
        throw std::runtime_error("Missing required fields");
    }

    creation.element_name = json_data.GetString("element_name");
    creation.title = json_data.GetString("title");
    creation.description = json_data.GetString("description");
    creation.image_data = json_data.GetString("image_data");
    creation.user_id = json_data.GetString("user_id"); // From authentication context

    if (json_data.KeyExists("tags"))
    {
        auto tags_array = json_data.GetArray("tags");
        for (size_t i = 0; i < tags_array.GetLength(); ++i)
        {
            creation.tags.push_back(tags_array[i].AsString());
        }
    }

    return creation;
}

Aws::Utils::Json::JsonValue CreationHandler::create_response(const Creation &creation)
{
    AWS_LOGSTREAM_INFO("CreateCreation", "Creating response for creation_id: " << creation.creation_id);

    try
    {
        // Get AWS region from environment
        Aws::String region = Aws::Environment::GetEnv("AWS_REGION");
        if (region.empty())
        {
            AWS_LOGSTREAM_ERROR("CreateCreation", "AWS_REGION environment variable not set");
            throw std::runtime_error("AWS_REGION not set");
        }

        // Construct S3 URLs
        std::string base_url = "https://" + bucket_name_ + ".s3." +
                               std::string(region.c_str()) + ".amazonaws.com/";

        Aws::Utils::Json::JsonValue response;
        response.WithString("creation_id", creation.creation_id)
            .WithString("element_name", creation.element_name)
            .WithString("title", creation.title)
            .WithString("image_url", base_url + creation.image_key)
            .WithString("thumbnail_url", base_url + creation.thumbnail_key)
            .WithString("creation_date", creation.creation_date);

        // Add tags if present
        if (!creation.tags.empty())
        {
            Aws::Utils::Array<Aws::Utils::Json::JsonValue> tagsArray(creation.tags.size());
            for (size_t i = 0; i < creation.tags.size(); ++i)
            {
                tagsArray[i].AsString(creation.tags[i]);
            }
            response.WithArray("tags", tagsArray);
        }

        AWS_LOGSTREAM_INFO("CreateCreation",
                           "Response created successfully for creation_id: "
                               << creation.creation_id);

        return response;
    }
    catch (const std::exception &e)
    {
        AWS_LOGSTREAM_ERROR("CreateCreation",
                            "Failed to create response: " << e.what());
        throw;
    }
}