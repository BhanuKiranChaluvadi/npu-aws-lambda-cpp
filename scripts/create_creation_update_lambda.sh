!/bin/bash

FUNCTION_NAME="create_creation"

echo "Updating Lambda function code..."
aws lambda update-function-code \
    --function-name $FUNCTION_NAME \
    --zip-file fileb://build/create_creation.zip

echo "Waiting for function update to complete..."
aws lambda wait function-updated \
    --function-name $FUNCTION_NAME

echo "Update complete!"