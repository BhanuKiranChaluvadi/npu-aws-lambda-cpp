#!/bin/bash

# Set variables
BUCKET_NAME="npu-creations-images-2025"
REGION="eu-north-1"

# Check if AWS CLI is installed
if ! command -v aws &> /dev/null
then
    echo "AWS CLI could not be found. Please install it."
    exit 1
fi

# 1. Create S3 Bucket
echo "Creating S3 bucket: $BUCKET_NAME in $REGION..."
aws s3api create-bucket \
    --bucket "$BUCKET_NAME" \
    --region "$REGION" \
    --create-bucket-configuration LocationConstraint="$REGION"

# 2. Configure Bucket Policy
echo "Applying bucket policy..."
aws s3api put-bucket-policy \
    --bucket "$BUCKET_NAME" \
    --policy file://bucket-policy.json

# Disable Block Public Access
echo "Disabling Block Public Access..."
aws s3api put-public-access-block \
    --bucket "$BUCKET_NAME" \
    --public-access-block-configuration "BlockPublicAcls=false,IgnorePublicAcls=false,BlockPublicPolicy=false,RestrictPublicBuckets=false"

# 3. Enable CORS
echo "Enabling CORS..."
aws s3api put-bucket-cors \
    --bucket "$BUCKET_NAME" \
    --cors-configuration file://cors-policy.json

# 4. Create Folder Structure
echo "Creating folder structure..."
aws s3api put-object --bucket "$BUCKET_NAME" --key uploads/
aws s3api put-object --bucket "$BUCKET_NAME" --key thumbnails/

echo "S3 bucket setup complete."