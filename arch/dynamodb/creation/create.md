# Step by Step Guide: Setting Up DynamoDB Table

## Prerequisites
```bash
# Install AWS CLI
sudo apt-get update
sudo apt-get install awscli

# Configure AWS credentials
aws configure
# Enter your:
# - AWS Access Key ID
# - AWS Secret Access Key
# - Default region (e.g., eu-north-1)
# - Default output format (json)
```

## 1. Create Table Definition
```json
// filepath: /home/nth/ws/npu-aws-lambda-cpp/setup/create-table.json
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

## 2. Create Table
```bash
# Create the table using the definition file
aws dynamodb create-table --cli-input-json file://setup/create-table.json

# Wait for table to become active
aws dynamodb wait table-exists --table-name NPUCreations
```

## 3. Insert Test Item
```bash
# Create a test item
aws dynamodb put-item \
    --table-name NPUCreations \
    --item '{
        "creation_id": {"S": "create123"},
        "user_id": {"S": "user789"},
        "element_name": {"S": "brick-2x4"},
        "title": {"S": "Cool Spaceship"},
        "description": {"S": "Using 2x4 brick as mini screens"},
        "image_url": {"S": "https://s3.example.com/image.jpg"},
        "creation_date": {"S": "2025-02-22"},
        "scores": {
            "M": {
                "total_score": {"N": "450"},
                "vote_count": {"N": "100"}
            }
        },
        "tags": {
            "L": [
                {"S": "spaceship"},
                {"S": "sci-fi"}
            ]
        }
    }'
```

## 4. Verify Setup
```bash
# Check table status
aws dynamodb describe-table \
    --table-name NPUCreations \
    --query 'Table.TableStatus'

# Test query using primary key
aws dynamodb get-item \
    --table-name NPUCreations \
    --key '{
        "creation_id": {"S": "create123"},
        "user_id": {"S": "user789"}
    }'

# Test query using GSI
aws dynamodb query \
    --table-name NPUCreations \
    --index-name ElementNameIndex \
    --key-condition-expression "element_name = :name" \
    --expression-attribute-values '{":name":{"S":"brick-2x4"}}'
```

## 5. Optional: Delete Table
```bash
# If you need to start over
aws dynamodb delete-table --table-name NPUCreations
```