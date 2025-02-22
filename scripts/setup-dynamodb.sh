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