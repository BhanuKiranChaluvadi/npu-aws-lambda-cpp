#!/bin/bash

# This script sets up the entire create_creation stack, including:
# - IAM role and policies
# - Lambda function
# - API Gateway

# ==============================================================================
# Configuration
# ==============================================================================

# AWS Resource Names
FUNCTION_NAME="create_creation"
API_NAME="npu-creations-api"
TABLE_NAME="NPUCreations"
BUCKET_NAME="npu-creations-images-2025"

# AWS Region and Account ID
REGION="eu-north-1"
ACCOUNT_ID="242201308302" # Replace with your AWS account ID

# IAM Role Configuration
ROLE_NAME="npu-create-creation-role"
TRUST_POLICY="iam/policies/lambda-trust-policy.json"
FUNCTION_POLICY="iam/policies/create-creation-policy.json"

# Lambda Function Configuration
RUNTIME="provided.al2"
HANDLER="create_creation"
ZIP_FILE="fileb://build/create_creation.zip"
TIMEOUT="15"
MEMORY_SIZE="128"

# API Gateway Configuration
ROUTE_PATH="/api/creations"
STAGE_NAME="prod"

# ==============================================================================
# Check Prerequisites
# ==============================================================================

# Check if AWS CLI is installed
if ! command -v aws &> /dev/null
then
    echo "Error: AWS CLI could not be found. Please install it."
    exit 1
fi

# Check if required files exist
if [ ! -f "$TRUST_POLICY" ]; then
    echo "Error: Trust policy file not found: $TRUST_POLICY"
    exit 1
fi

if [ ! -f "$FUNCTION_POLICY" ]; then
    echo "Error: Function policy file not found: $FUNCTION_POLICY"
    exit 1
fi

if [ ! -f "$ZIP_FILE" ]; then
    echo "Error: Lambda function ZIP file not found: $ZIP_FILE"
    exit 1
fi

# ==============================================================================
# IAM Role Setup
# ==============================================================================

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

# ==============================================================================
# Lambda Function Setup
# ==============================================================================

# Create Lambda function
echo "Creating Lambda function: $FUNCTION_NAME..."
aws lambda create-function \
    --region "$REGION" \
    --function-name "$FUNCTION_NAME" \
    --role "$ROLE_ARN" \
    --runtime "$RUNTIME" \
    --timeout "$TIMEOUT" \
    --memory-size "$MEMORY_SIZE" \
    --handler "$HANDLER" \
    --zip-file "$ZIP_FILE"

# ==============================================================================
# API Gateway Setup
# ==============================================================================

# Create API
echo "Creating API Gateway: $API_NAME..."
API_ID=$(aws apigatewayv2 create-api \
    --name "$API_NAME" \
    --protocol-type HTTP \
    --target "arn:aws:lambda:$REGION:$ACCOUNT_ID:function:$FUNCTION_NAME" \
    --query 'ApiId' \
    --output text)

# Create Lambda integration
echo "Creating Lambda integration..."
INTEGRATION_ID=$(aws apigatewayv2 create-integration \
    --api-id "$API_ID" \
    --integration-type AWS_PROXY \
    --integration-uri "arn:aws:lambda:$REGION:$ACCOUNT_ID:function:$FUNCTION_NAME" \
    --payload-format-version "2.0" \
    --query 'IntegrationId' \
    --output text)

# Create route with the integration
echo "Creating route: POST $ROUTE_PATH..."
ROUTE_ID=$(aws apigatewayv2 create-route \
    --api-id "$API_ID" \
    --route-key "POST $ROUTE_PATH" \
    --target "integrations/$INTEGRATION_ID" \
    --query 'RouteId' \
    --output text)

# Add Lambda permission
echo "Adding Lambda permission..."
aws lambda add-permission \
    --function-name "$FUNCTION_NAME" \
    --statement-id api-gateway-permission \
    --action lambda:InvokeFunction \
    --principal apigateway.amazonaws.com \
    --source-arn "arn:aws:execute-api:$REGION:$ACCOUNT_ID:$API_ID/*"

# Create and deploy a stage
echo "Creating and deploying stage: $STAGE_NAME..."
aws apigatewayv2 create-stage \
    --api-id "$API_ID" \
    --stage-name "$STAGE_NAME" \
    --auto-deploy

# Get and save API URL
echo "Getting API endpoint..."
API_URL=$(aws apigatewayv2 get-api \
    --api-id "$API_ID" \
    --query 'ApiEndpoint' \
    --output text)

echo "API Endpoint: $API_URL/$STAGE_NAME$ROUTE_PATH"
echo "$API_URL/$STAGE_NAME$ROUTE_PATH" > api-endpoint.txt

# ==============================================================================
# Lambda Environment Variables
# ==============================================================================

# Update Lambda function configuration
echo "Updating Lambda function configuration..."
aws lambda update-function-configuration \
    --function-name "$FUNCTION_NAME" \
    --environment "Variables={BUCKET_NAME=$BUCKET_NAME,TABLE_NAME=$TABLE_NAME,AWS_REGION=$REGION}"

# ==============================================================================
# Completion
# ==============================================================================

echo "create_creation stack setup complete."
echo " - API Endpoint: $API_URL/$STAGE_NAME$ROUTE_PATH"
echo " - Role ARN: $ROLE_ARN"
echo " - API ID: $API_ID"
echo "Remember to configure your DynamoDB and S3 buckets separately."