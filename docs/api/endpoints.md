# NPU Platform API Endpoints

## 1. Create NPU Creation
```http
POST /api/creations
Content-Type: multipart/form-data

{
    "element_name": "string",
    "title": "string",
    "description": "string",
    "image": "file",
    "tags": ["string"]
}

Response: {
    "creation_id": "string",
    "image_url": "string",
    "thumbnail_url": "string",
    "creation_date": "string"
}
```

## 2. Get Creation Details
```http
GET /api/creations/{creation_id}

Response: {
    "creation_id": "string",
    "user_id": "string",
    "element_name": "string",
    "title": "string",
    "description": "string",
    "image_url": "string",
    "thumbnail_url": "string",
    "creation_date": "string",
    "scores": {
        "total_score": number,
        "vote_count": number
    },
    "tags": ["string"]
}
```

## 3. Search Creations by Element
```http
GET /api/elements/{element_name}/creations
Query Parameters:
- page_size: number
- last_evaluated_key: string (for pagination)

Response: {
    "items": [{
        "creation_id": "string",
        "title": "string",
        "thumbnail_url": "string",
        "scores": {
            "total_score": number,
            "vote_count": number
        }
    }],
    "last_evaluated_key": "string"
}
```

## 4. Submit Score
```http
POST /api/creations/{creation_id}/score
{
    "score": number (1-10)
}

Response: {
    "total_score": number,
    "vote_count": number
}
```

## 5. List Recent Creations
```http
GET /api/creations
Query Parameters:
- page_size: number
- last_evaluated_key: string

Response: {
    "items": [{
        "creation_id": "string",
        "title": "string",
        "element_name": "string",
        "thumbnail_url": "string",
        "creation_date": "string",
        "scores": {
            "total_score": number,
            "vote_count": number
        }
    }],
    "last_evaluated_key": "string"
}
```

## 6. Get Upload URL (Presigned)
```http
GET /api/upload/presigned
Query Parameters:
- file_name: string

Response: {
    "upload_url": "string",
    "image_key": "string"
}
```

## Implementation Notes

### DynamoDB Operations
```cpp
// Example DynamoDB query for element search
{
    "TableName": "NPUCreations",
    "IndexName": "ElementNameIndex",
    "KeyConditionExpression": "element_name = :element",
    "ExpressionAttributeValues": {
        ":element": {"S": "brick-2x4"}
    },
    "Limit": 20
}
```

### S3 Operations
```cpp
// Image storage paths
uploads/{creation_id}/main.jpg      // Original image
thumbnails/{creation_id}/thumb.jpg   // Thumbnail
```

### Security Considerations
- Use presigned URLs for image uploads
- Validate file types and sizes
- Authenticate users for write operations
- Rate limit score submissions