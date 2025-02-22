# DynamoDB Setup Guide

This guide provides step-by-step instructions for setting up a DynamoDB table to store creation data for the NPU Creations platform.

## Prerequisites

Ensure you have the following before proceeding:
- AWS CLI installed and configured.

## 1. Create Table Definition

### Create a Table Definition File
Create a file named `create-table.json` with the following content:

```json
{
    "TableName": "NPUCreations",
    "AttributeDefinitions": [
        {
            "AttributeName": "creation_id",
            "AttributeType": "S"
        },
        {
            "AttributeName": "user_id",
            "AttributeType": "S"
        },
        {
            "AttributeName": "element_name",
            "AttributeType": "S"
        },
        {
            "AttributeName": "creation_date",
            "AttributeType": "S"
        }
    ],
    "KeySchema": [
        {
            "AttributeName": "creation_id",
            "KeyType": "HASH"
        },
        {
            "AttributeName": "user_id",
            "KeyType": "RANGE"
        }
    ],
    "GlobalSecondaryIndexes": [
        {
            "IndexName": "ElementNameIndex",
            "KeySchema": [
                {
                    "AttributeName": "element_name",
                    "KeyType": "HASH"
                },
                {
                    "AttributeName": "creation_date",
                    "KeyType": "RANGE"
                }
            ],
            "Projection": {
                "ProjectionType": "ALL"
            },
            "ProvisionedThroughput": {
                "ReadCapacityUnits": 5,
                "WriteCapacityUnits": 5
            }
        }
    ],
    "ProvisionedThroughput": {
        "ReadCapacityUnits": 5,
        "WriteCapacityUnits": 5
    }
}
```

## 2. Create the DynamoDB Table

Run the following command to create the table:
```bash
aws dynamodb create-table --cli-input-json file://create-table.json
```

## 3. Wait for Table Creation

```bash
aws dynamodb wait table-exists --table-name NPUCreations
```

## 4. Table Structure

### Main Table: `NPUCreations`

| Attribute       | Type   | Key Type | Description                            |
|---------------|--------|----------|--------------------------------|
| creation_id   | String | HASH     | Unique identifier for the creation   |
| user_id       | String | RANGE    | User who created the item            |
| element_name  | String |          | Name of the LEGO element used        |
| title         | String |          | Title of the creation                |
| description   | String |          | Description of the creation          |
| image_url     | String |          | URL of the image                     |
| creation_date | String |          | Date the creation was uploaded       |
| scores        | Map    |          | Contains `total_score` and `vote_count` |
| tags          | List   |          | List of tags associated with the creation |

### Global Secondary Index: `ElementNameIndex`

| Attribute       | Type   | Key Type | Description                            |
|---------------|--------|----------|--------------------------------|
| element_name  | String | HASH     | Name of the LEGO element used        |
| creation_date | String | RANGE    | Date the creation was uploaded       |

## 5. Important Notes

- `creation_id` is the **partition key (HASH)**.
- `user_id` is the **sort key (RANGE)**.
- `ElementNameIndex` allows efficient queries for creations using a specific LEGO element.
- Only define attributes in `AttributeDefinitions` if they are used as keys (primary or secondary).
- Consider using `BillingMode: PAY_PER_REQUEST` for unpredictable workloads.

## 6. Automated Setup Script

Save the following script as `setup-dynamodb.sh`:

```bash
#!/bin/bash

# Set variables
TABLE_NAME="NPUCreations"
REGION="eu-north-1"

# Check if AWS CLI is installed
if ! command -v aws &> /dev/null
then
    echo "AWS CLI could not be found. Please install it."
    exit 1
fi

# 1. Create Table
echo "Creating DynamoDB table: $TABLE_NAME..."
aws dynamodb create-table --cli-input-json file://create-table.json

# 2. Wait for Table Creation
echo "Waiting for table to be created..."
aws dynamodb wait table-exists --table-name "$TABLE_NAME"

echo "DynamoDB table setup complete."
```

## 7. Next Steps

1. Place `create-table.json` in the same directory as `setup-dynamodb.sh` or update the script's file path.
2. Make the script executable:
   ```bash
   chmod +x setup-dynamodb.sh
   ```
3. Run the script:
   ```bash
   ./setup-dynamodb.sh
   ```
4. Verify the table creation in the AWS console.

> **Tip:** Before deploying to production, review your table definition, provisioned throughput, and indexing needs. Consider using **BillingMode: PAY_PER_REQUEST** for unpredictable workloads.

