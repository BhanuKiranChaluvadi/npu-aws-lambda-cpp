# First, base64 encode the image
IMAGE_DATA=$(base64 arch/bucket/npu-samples/tree.jpg)

# Then, construct the JSON payload with the base64 encoded image data
JSON_PAYLOAD=$(printf '{
    "element_name": "test_element",
    "title": "Test Creation",
    "description": "A test creation",
    "image_data": "%s",
    "user_id": "test_user",
    "tags": ["test", "demo"]
}' "$IMAGE_DATA")

# Finally, make the curl request
curl -v -X POST \
  "https://fpbnbxu78l.execute-api.eu-north-1.amazonaws.com/default/create_creation" \
  -H "Content-Type: application/json" \
  -d "$JSON_PAYLOAD"