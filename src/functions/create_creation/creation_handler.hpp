#pragma once
#include <aws/lambda-runtime/runtime.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include "../../common/services/dynamodb_service.hpp"
#include "../../common/services/s3_service.hpp"
#include "../../common/models/creation.hpp"

class CreationHandler
{
public:
    CreationHandler(
        const DynamoDBService &dynamo_service,
        const S3Service &s3_service,
        const std::string &bucket_name);

    aws::lambda_runtime::invocation_response handle_request(
        Creation &creation);

    Creation parse_request(const Aws::String &request_payload);

private:
    Aws::Utils::Json::JsonValue create_response(const Creation &creation);

    const DynamoDBService& dynamo_service_;
    const S3Service& s3_service_;

    std::string bucket_name_;

    static const char* TAG;
};