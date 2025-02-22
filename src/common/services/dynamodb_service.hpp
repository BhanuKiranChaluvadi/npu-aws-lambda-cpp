#pragma once
#include <aws/dynamodb/DynamoDBClient.h>
#include "../models/creation.hpp"
#include <aws/core/utils/logging/LogMacros.h>

class DynamoDBService
{
public:
    /**
     * @brief Construct a new DynamoDB Service
     * @param client Reference to AWS DynamoDB client
     * @param table_name Name of the DynamoDB table
     * @throws None Constructor is noexcept
     */
    explicit DynamoDBService(
        const Aws::DynamoDB::DynamoDBClient &client,
        std::string_view table_name) noexcept;

    /**
     * @brief Save a creation to DynamoDB
     * @param creation The creation to save
     * @return true if successful, false otherwise
     * @throws None Method handles all exceptions internally
     */
    bool save_creation(const Creation &creation) const;

private:
    const Aws::DynamoDB::DynamoDBClient &client_;
    const std::string table_name_;
};