#include "dynamodb_service.hpp"
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/core/utils/logging/LogMacros.h>

DynamoDBService::DynamoDBService(
    const Aws::DynamoDB::DynamoDBClient &client,
    std::string_view table_name) noexcept
    : client_(client), table_name_(table_name) {}

bool DynamoDBService::save_creation(const Creation &creation) const
{
    // Validate input
    if (!creation.validate())
    {
        AWS_LOGSTREAM_ERROR("DynamoDBService",
                            "Invalid creation data for ID: " << creation.creation_id);
        return false;
    }

    try
    {
        Aws::DynamoDB::Model::PutItemRequest request;
        request.SetTableName(table_name_);

        // Create attribute map
        Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue> item;

        // Add required fields
        item["creation_id"].SetS(creation.creation_id);
        item["user_id"].SetS(creation.user_id);
        item["element_name"].SetS(creation.element_name);
        item["title"].SetS(creation.title);
        item["description"].SetS(creation.description);
        item["image_key"].SetS(creation.image_key);
        item["thumbnail_key"].SetS(creation.thumbnail_key);
        item["creation_date"].SetS(creation.creation_date);

        // Add tags if present
        if (!creation.tags.empty())
        {
            Aws::Vector<std::shared_ptr<Aws::DynamoDB::Model::AttributeValue>> tag_list;
            tag_list.reserve(creation.tags.size()); // Optimize vector growth

            for (const auto &tag : creation.tags)
            {
                auto tag_av = Aws::MakeShared<Aws::DynamoDB::Model::AttributeValue>("TagAttribute");
                tag_av->SetS(tag);
                tag_list.push_back(tag_av);
            }
            item["tags"].SetL(std::move(tag_list)); // Use move semantics
        }

        // Initialize scores map
        Aws::Map<Aws::String, const std::shared_ptr<Aws::DynamoDB::Model::AttributeValue>> scores;
        const auto total_score = Aws::MakeShared<Aws::DynamoDB::Model::AttributeValue>("TotalScore");
        const auto vote_count = Aws::MakeShared<Aws::DynamoDB::Model::AttributeValue>("VoteCount");
        total_score->SetN("0");
        vote_count->SetN("0");
        scores.emplace("total_score", total_score);
        scores.emplace("vote_count", vote_count);
        item["scores"].SetM(scores);

        request.SetItem(item);

        // Execute the request
        const auto outcome = client_.PutItem(request);

        if (!outcome.IsSuccess())
        {
            AWS_LOGSTREAM_ERROR("DynamoDBService",
                                "Failed to save creation: " << outcome.GetError().GetMessage());
            return false;
        }

        AWS_LOGSTREAM_INFO("DynamoDBService",
                           "Successfully saved creation with ID: " << creation.creation_id);
        return true;
    }
    catch (const std::exception &e)
    {
        AWS_LOGSTREAM_ERROR("DynamoDBService",
                            "Exception while saving creation: " << e.what());
        return false;
    }
}