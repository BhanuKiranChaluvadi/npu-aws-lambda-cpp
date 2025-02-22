#include <aws/core/Aws.h>
#include <aws/lambda-runtime/runtime.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/platform/Environment.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/dynamodb/model/DescribeTableRequest.h>
#include <aws/dynamodb/model/ListTablesRequest.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/memory/stl/SimpleStringStream.h>
#include "../../common/services/dynamodb_service.hpp"
#include "../../common/services/s3_service.hpp"
#include "creation_handler.hpp"

namespace
{
    constexpr char TAG[] = "NPUCreations";
    constexpr char ENV_BUCKET_NAME[] = "BUCKET_NAME";
    constexpr char ENV_TABLE_NAME[] = "TABLE_NAME";
    constexpr char ENV_AWS_REGION[] = "AWS_REGION";

    std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()>
    GetConsoleLoggerFactory(Aws::Utils::Logging::LogLevel level)
    {
        return [level]
        {
            return Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>(
                "console_logger", level);
        };
    }

    Aws::Client::ClientConfiguration CreateClientConfig()
    {
        Aws::Client::ClientConfiguration config;
        config.region = Aws::Environment::GetEnv(ENV_AWS_REGION);
        config.caFile = "/etc/pki/tls/certs/ca-bundle.crt";
        config.disableExpectHeader = true;
        config.connectTimeoutMs = 5000;  // 5 second connection timeout
        config.requestTimeoutMs = 10000; // 10 second request timeout
        return config;
    }
}

using namespace aws::lambda_runtime;

// Modified handler function to return invocation_response
invocation_response my_handler(invocation_request const &request)
{
    using namespace Aws::Utils::Json;

    try
    {
        // Get environment variables
        const char *bucket_name = std::getenv(ENV_BUCKET_NAME);
        const char *table_name = std::getenv(ENV_TABLE_NAME);
        const char *region = std::getenv(ENV_AWS_REGION);

        if (!bucket_name || !table_name || !region)
        {
            throw std::runtime_error("Required environment variables not set");
        }

        // Configure and create AWS clients
        auto config = CreateClientConfig();
        auto credentials_provider = Aws::MakeShared<Aws::Auth::EnvironmentAWSCredentialsProvider>(TAG);

        Aws::S3::S3Client s3_client(config);
        Aws::DynamoDB::DynamoDBClient dynamo_client(credentials_provider, config);

        // Create services and handler
        static S3Service s3_service(s3_client, bucket_name);
        static DynamoDBService dynamo_service(dynamo_client, table_name);
        static CreationHandler handler(dynamo_service, s3_service, bucket_name);
        AWS_LOGSTREAM_INFO(TAG, "Initialized AWS Services");

        AWS_LOGSTREAM_INFO(TAG, "Handling request: " << request.request_id);
        AWS_LOGSTREAM_DEBUG(TAG, "Request payload: " << request.payload);

        // Parse the request
        Creation creation = handler.parse_request(request.payload);

        // Call the existing handler and get the response
        auto response = handler.handle_request(creation);

        // Convert the response to a JSON string
        JsonValue resp_json;
        resp_json.WithString("message", response.get_payload());

        return invocation_response::success(resp_json.View().WriteCompact(), "application/json");
    }
    catch (const std::exception &e)
    {
        AWS_LOGSTREAM_ERROR(TAG, "Fatal error: " << e.what());
        return invocation_response::failure(e.what(), "Exception");
    }
    catch (...)
    {
        AWS_LOGSTREAM_ERROR(TAG, "Unknown fatal error");
        return invocation_response::failure("Unknown error", "UnknownException");
    }
}

int main()
{
    // Initialize AWS SDK
    Aws::SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory(Aws::Utils::Logging::LogLevel::Trace);
    Aws::InitAPI(options);
    AWS_LOGSTREAM_INFO(TAG, "AWS SDK initialized");

    // Run the handler
    run_handler(my_handler);

    // Shutdown AWS SDK
    Aws::ShutdownAPI(options);
    return 0;
}