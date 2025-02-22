# AWS IAM Setup Guide for Lambda and DynamoDB

This guide provides step-by-step instructions to create IAM roles and policies required for Lambda functions interacting with DynamoDB and S3.

## Prerequisites

Ensure you have the following before proceeding:
- AWS CLI installed and configured.
- Proper permissions to create IAM roles and policies.

## 1. Create IAM Policies

### Lambda Trust Policy
Create a file named `iam/policies/lambda-trust-policy.json` with the following content:

```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Principal": {
                "Service": "lambda.amazonaws.com"
            },
            "Action": "sts:AssumeRole"
        }
    ]
}
```

### Custom Lambda Execution Policy
Create a file named `iam/policies/create-creation-policy.json` with the following content:

```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "logs:CreateLogGroup",
                "logs:CreateLogStream",
                "logs:PutLogEvents"
            ],
            "Resource": "arn:aws:logs:eu-north-1:242201308302:*"
        },
        {
            "Effect": "Allow",
            "Action": [
                "dynamodb:PutItem",
                "dynamodb:GetItem",
                "dynamodb:Query"
            ],
            "Resource": "arn:aws:dynamodb:eu-north-1:242201308302:table/NPUCreations"
        },
        {
            "Effect": "Allow",
            "Action": [
                "s3:PutObject",
                "s3:GetObject",
                "s3:DeleteObject"
            ],
            "Resource": "arn:aws:s3:::npu-creations-images-2025/*"
        }
    ]
}
```

## 2. Setup IAM Role

Save the following script as `scripts/setup-iam-role.sh`:

```bash
#!/bin/bash

# Set variables
ROLE_NAME="npu-create-creation-role"
TRUST_POLICY="iam/policies/lambda-trust-policy.json"
FUNCTION_POLICY="iam/policies/create-creation-policy.json"
REGION="eu-north-1"
ACCOUNT_ID="242201308302"

# Check if AWS CLI is installed
if ! command -v aws &> /dev/null
then
    echo "AWS CLI could not be found. Please install it."
    exit 1
fi

# Create the IAM role
echo "Creating IAM role: $ROLE_NAME..."
aws iam create-role \
    --role-name "$ROLE_NAME" \
    --assume-role-policy-document file://$TRUST_POLICY

# Attach AWS managed policy for basic Lambda execution (CloudWatch Logs)
echo "Attaching AWSLambdaBasicExecutionRole policy..."
aws iam attach-role-policy \
    --role-name "$ROLE_NAME" \
    --policy-arn arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole

# Attach the custom policy to the role
echo "Attaching custom policy: $FUNCTION_POLICY..."
aws iam put-role-policy \
    --role-name "$ROLE_NAME" \
    --policy-name npu-create-creation-policy \
    --policy-document file://$FUNCTION_POLICY

# Get and store the role ARN
echo "Getting and storing role ARN..."
ROLE_ARN=$(aws iam get-role --role-name "$ROLE_NAME" --query 'Role.Arn' --output text)
echo "Role ARN: $ROLE_ARN"
echo "$ROLE_ARN" > role-arn.txt

echo "IAM role setup complete. Role ARN saved to role-arn.txt"
```
