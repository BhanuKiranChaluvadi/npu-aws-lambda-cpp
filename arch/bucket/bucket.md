# Setting Up S3 for NPU Image Storage

## 1. Create S3 Bucket
```bash
# Create bucket (replace UNIQUE_BUCKET_NAME with your unique bucket name)
# Note: S3 bucket names must be globally unique across all AWS accounts
aws s3api create-bucket \
    --bucket npu-creations-images-2025 \
    --region eu-north-1 \
    --create-bucket-configuration LocationConstraint=eu-north-1
```

## 2. Configure Bucket Policy
```json
// filepath: /home/nth/ws/npu-aws-lambda-cpp/setup/bucket-policy.json
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

```bash
#[WARNING] Disable Block Public Access settings for the bucket - 
aws s3api put-public-access-block \
    --bucket npu-creations-images-2025 \
    --public-access-block-configuration "BlockPublicAcls=false,IgnorePublicAcls=false,BlockPublicPolicy=false,RestrictPublicBuckets=false"


# Apply the bucket policy
aws s3api put-bucket-policy \
    --bucket npu-creations-images-2025 \
    --policy file://setup/bucket-policy.json
```

## 3. Enable CORS for Web Access
```json
// filepath: /home/nth/ws/npu-aws-lambda-cpp/setup/cors-policy.json
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

```bash
# Apply CORS configuration
aws s3api put-bucket-cors \
    --bucket npu-creations-images-2025 \
    --cors-configuration file://setup/cors-policy.json
```

## 4. Test Upload
```bash
# Create a test image
echo "Test Image" > test-image.jpg

# Upload test image
aws s3 cp test-image.jpg s3://npu-creations-images-2025/test/test-image.jpg

# Verify upload
aws s3 ls s3://npu-creations-images-2025/test/
```

## 5. Create Folder Structure
```bash
# Create folders for organizing uploads
aws s3api put-object --bucket npu-creations-images-2025 --key uploads/
aws s3api put-object --bucket npu-creations-images-2025 --key thumbnails/
```

## 6. Clean Up (if needed)
```bash
# Remove all objects
aws s3 rm s3://npu-creations-images-2025 --recursive

# Delete bucket
aws s3api delete-bucket --bucket npu-creations-images-2025
```

## Important Notes:
1. **Security**:
   - The bucket policy allows public read access for viewing images
   - Write access should be restricted to authenticated users
   - Consider using presigned URLs for uploads

2. **Image Organization**:
   - Images stored as: `uploads/[creation_id]/image.jpg`
   - Thumbnails stored as: `thumbnails/[creation_id]/thumb.jpg`

3. **Integration with DynamoDB**:
   ```json
   // Example DynamoDB item with S3 references
   {
       "creation_id": {"S": "create123"},
       "image_key": {"S": "uploads/create123/main.jpg"},
       "thumbnail_key": {"S": "thumbnails/create123/thumb.jpg"}
   }
   ```

# DynamoDB and S3 Integration

## DynamoDB Item Structure
```json
{
    "creation_id": {"S": "create123"},
    "user_id": {"S": "user789"},
    "element_name": {"S": "brick-2x4"},
    "image_key": {"S": "uploads/create123/main.jpg"},    // Just the S3 key
    "thumbnail_key": {"S": "thumbnails/create123/thumb.jpg"}  // Just the S3 key
}
```

## Why This Works
1. The S3 bucket name is configured in your application
2. Full S3 URL can be constructed when needed:
   ```cpp
   std::string getImageUrl(const std::string& image_key) {
       return "https://" + BUCKET_NAME + ".s3." + REGION + ".amazonaws.com/" + image_key;
   }
   ```

## Benefits
- Saves storage space in DynamoDB
- More flexible if bucket name changes
- Easier to migrate between regions
- Simpler to manage in code

# Understanding Thumbnail Keys in NPU Platform

## What is a thumbnail_key?
- A reference to a smaller, optimized version of the original image
- Stored in a separate `thumbnails/` folder in S3
- Used for preview listings and mobile applications

## Example Structure
```json
{
    "creation_id": {"S": "create123"},
    "image_key": {"S": "uploads/create123/main.jpg"},     // Original high-res image
    "thumbnail_key": {"S": "thumbnails/create123/thumb.jpg"}  // Smaller preview image
}
```

## Why Use Thumbnails?
1. **Performance Benefits**:
   - Faster loading in image grids/lists
   - Reduced bandwidth usage for mobile users
   - Better user experience when browsing multiple NPU creations

2. **Storage Organization**:
```plaintext
S3 Bucket Structure:
└── npu-creations-images-2025/
    ├── uploads/
    │   └── create123/
    │       └── main.jpg (original: 2048x1536px, 2MB)
    └── thumbnails/
        └── create123/
            └── thumb.jpg (thumbnail: 300x225px, 20KB)
```

3. **Usage Scenarios**:
   - List view: Uses thumbnails
   - Grid view: Uses thumbnails
   - Detail view: Uses original image