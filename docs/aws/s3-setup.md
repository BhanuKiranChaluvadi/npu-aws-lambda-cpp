# S3 Setup Guide

This guide provides step-by-step instructions for setting up an S3 bucket to store images and thumbnails for the NPU Creations platform.

## Prerequisites

Ensure you have the following before proceeding:
- AWS CLI installed and configured.
- A globally unique S3 bucket name.

## 1. Create an S3 Bucket

```bash
aws s3api create-bucket \
    --bucket npu-creations-images-2025 \
    --region eu-north-1 \
    --create-bucket-configuration LocationConstraint=eu-north-1
```

> **Note:** S3 bucket names must be globally unique across all AWS accounts. Replace `npu-creations-images-2025` with your unique bucket name.

## 2. Configure Bucket Policy

### Create a Bucket Policy File
Create a file named `bucket-policy.json` with the following content:

```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Sid": "PublicReadForGetBucketObjects",
            "Effect": "Allow",
            "Principal": "*",
            "Action": "s3:GetObject",
            "Resource": "arn:aws:s3:::npu-creations-images-2025/*"
        }
    ]
}
```

### Apply the Bucket Policy
```bash
aws s3api put-bucket-policy \
    --bucket npu-creations-images-2025 \
    --policy file://bucket-policy.json
```

> **Warning:** This bucket policy allows public read access for images. Restrict write access to authenticated users. Consider using **presigned URLs** for uploads.

### Disable Block Public Access
```bash
aws s3api put-public-access-block \
    --bucket npu-creations-images-2025 \
    --public-access-block-configuration "BlockPublicAcls=false,IgnorePublicAcls=false,BlockPublicPolicy=false,RestrictPublicBuckets=false"
```

## 3. Enable CORS for Web Access

### Create a CORS Policy File
Create a file named `cors-policy.json` with the following content:

```json
{
    "CORSRules": [
        {
            "AllowedHeaders": ["*"],
            "AllowedMethods": ["GET", "PUT", "POST"],
            "AllowedOrigins": ["*"],
            "ExposeHeaders": ["ETag"],
            "MaxAgeSeconds": 3000
        }
    ]
}
```

### Apply CORS Configuration
```bash
aws s3api put-bucket-cors \
    --bucket npu-creations-images-2025 \
    --cors-configuration file://cors-policy.json
```

## 4. Create Folder Structure

To organize images and thumbnails, create the following folders:
```bash
aws s3api put-object --bucket npu-creations-images-2025 --key uploads/
aws s3api put-object --bucket npu-creations-images-2025 --key thumbnails/
```

### File Storage Structure
- Images are stored as: `uploads/[creation_id]/image.jpg`
- Thumbnails are stored as: `thumbnails/[creation_id]/thumb.jpg`

## 5. Integration with DynamoDB

### Example DynamoDB Item Structure

```json
{
    "creation_id": {"S": "create123"},
    "image_key": {"S": "uploads/create123/main.jpg"},
    "thumbnail_key": {"S": "thumbnails/create123/thumb.jpg"}
}
```

## 6. Constructing S3 URLs in Your Application

Your application should be configured with the S3 bucket name. The full S3 URL can be constructed as follows:

```cpp
std::string getImageUrl(const std::string& image_key) {
    return "https://" + BUCKET_NAME + ".s3." + REGION + ".amazonaws.com/" + image_key;
}
```