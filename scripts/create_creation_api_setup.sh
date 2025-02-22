#!/bin/bash

# Set variables
REGION="eu-north-1"
ACCOUNT_ID="242201308302"
FUNCTION_NAME="create_creation"
API_NAME="npu-creations-api"
ROUTE_PATH="/api/creations"
STAGE_NAME="prod"
BUCKET_NAME="npu-creations-images-2025"
TABLE_NAME="NPUCreations"

# Check if AWS CLI is installed
if ! command -v aws &> /dev/null
then
    echo "AWS CLI could not be found. Please install it."
    exit 1
fi

# Create API
echo "Creating API Gateway..."
API_ID=$(aws apigatewayv2 create-api \
    --name "$API_NAME" \
    --protocol-type HTTP \
    --target "arn:aws:lambda:$REGION:$ACCOUNT_ID:function:$FUNCTION_NAME" \
    --query 'ApiId' \
    --output text)

echo "API Gateway ID: $API_ID"

# Create Lambda integration
echo "Creating Lambda integration..."
INTEGRATION_ID=$(aws apigatewayv2 create-integration \
    --api-id "$API_ID" \
    --integration-type AWS_PROXY \
    --integration-uri "arn:aws:lambda:$REGION:$ACCOUNT_ID:function:$FUNCTION_NAME" \
    --payload-format-version "2.0" \
    --query 'IntegrationId' \
    --output text)

echo "Integration ID: $INTEGRATION_ID"

# Create route with the integration
echo "Creating route: POST $ROUTE_PATH..."
ROUTE_ID=$(aws apigatewayv2 create-route \
    --api-id "$API_ID" \
    --route-key "POST $ROUTE_PATH" \
    --target "integrations/$INTEGRATION_ID" \
    --query 'RouteId' \
    --output text)

echo "Route ID: $ROUTE_ID"

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

echo "API Endpoint: $API_URL$ROUTE_PATH"
echo "$API_URL$ROUTE_PATH" > api-endpoint.txt

# Save important IDs for future reference
echo "Saving API configuration..."
echo "API_ID=$API_ID" > api-config.txt
echo "INTEGRATION_ID=$INTEGRATION_ID" >> api-config.txt
echo "ROUTE_ID=$ROUTE_ID" >> api-config.txt
echo "API_URL=$API_URL" >> api-config.txt

# Update Lambda function configuration
echo "Updating Lambda function configuration..."
aws lambda update-function-configuration \
    --function-name "$FUNCTION_NAME" \
    --environment "Variables={BUCKET_NAME=$BUCKET_NAME,TABLE_NAME=$TABLE_NAME,AWS_REGION=$REGION}"

# Test the endpoint (optional)
echo "Testing endpoint..."
curl -X POST "$API_URL/$STAGE_NAME$ROUTE_PATH" \
  -H "Content-Type: application/json" \
  -d '{
    "element_name": "test_element",
    "title": "Test Creation",
    "description": "A test creation",
    "image_data": "base64_encoded_image_data",
    "user_id": "test_user",
    "tags": ["test", "demo"]
  }'

echo "API setup complete."