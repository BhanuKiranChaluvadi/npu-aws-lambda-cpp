# AWS Lambda C++ Deployment Guide

## Prerequisites

Ensure you have the following installed before proceeding:
- Docker
- AWS CLI
- CMake
- GCC & Development Tools
- Git

## Setup Amazon Linux Environment

### Pull and Run Amazon Linux Docker Container
```bash
# Pull Amazon Linux Docker image
docker pull amazonlinux:latest

# Start a container
docker run -it amazonlinux:latest /bin/bash
```

### Install Required Packages
```bash
yum update -y
yum install -y nano wget unzip tar
yum install -y openssl-devel cmake git make zip libcurl-devel
yum groupinstall -y "Development Tools"
yum install -y gcc gcc-c++ tree zlib-devel
```

## Build AWS SDK for C++

```bash
mkdir ~/install

# Clone AWS SDK for C++ repository
git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
cd aws-sdk-cpp

# Create and navigate to the build directory
mkdir build && cd build

# Configure and compile
cmake .. -DBUILD_ONLY="core;dynamodb;s3" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCUSTOM_MEMORY_MANAGEMENT=OFF \
    -DCMAKE_INSTALL_PREFIX=~/install

make -j$(nproc)
make install
```

## Build AWS Lambda C++ Runtime
```bash
# Clone AWS Lambda C++ repository
git clone https://github.com/awslabs/aws-lambda-cpp.git
cd aws-lambda-cpp

# Create and navigate to the build directory
mkdir build && cd build

# Configure and compile
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=~/install

make -j$(nproc)
make install
```

## Fixing Compilation Issues

If you encounter a ZLIB error, modify the CMake configuration in your examples:

```cmake
find_package(ZLIB REQUIRED)  # Ensure ZLIB is found
find_package(AWSSDK COMPONENTS core)

target_link_libraries(${PROJECT_NAME} PUBLIC \
    AWS::aws-lambda-runtime \
    ${AWSSDK_LINK_LIBRARIES} \
    ZLIB::ZLIB)
```

## Deploying AWS Lambda Functions

### Create IAM Role for Lambda

```bash
# Define IAM trust policy
cat > trust-policy.json << EOF
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Principal": {
        "Service": ["lambda.amazonaws.com"]
      },
      "Action": "sts:AssumeRole"
    }
  ]
}
EOF

# Create IAM Role (Note the ARN)
aws iam create-role \
  --role-name lambda-demo \
  --assume-role-policy-document file://trust-policy.json

# Attach CloudWatch Logging Policy
aws iam attach-role-policy \
  --role-name lambda-demo \
  --policy-arn arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole
```

### Deploy Lambda Function
```bash
aws lambda create-function --function-name demo \
    --role arn:aws:iam::<ACCOUNT_ID>:role/lambda-demo \
    --runtime provided.al2 \
    --timeout 15 \
    --memory-size 128 \
    --handler hello \
    --zip-file fileb://hello.zip
```

### Invoke Lambda Function
```bash
aws lambda invoke \
    --function-name demo \
    --payload fileb://payload.json \
    output.txt
```

## API Gateway Integration

### Deploy Lambda Function for API Gateway
```bash
aws lambda create-function \
    --function-name cpp-api-gateway-demo \
    --role arn:aws:iam::<ACCOUNT_ID>:role/lambda-demo \
    --runtime provided.al2 \
    --timeout 15 \
    --memory-size 128 \
    --handler api \
    --zip-file fileb://api.zip
```

### Invoke API Gateway Endpoint
```bash
curl -v -X POST \
    'https://<api-id>.execute-api.<region>.amazonaws.com/default/cpp-api-gateway-demo?name=Bradley&city=Chicago' \
    -H 'Content-Type: application/json' \
    -H 'day: Sunday' \
    -d '{ "time": "evening" }'
```

## Using AWS Lambda with DynamoDB

### Deploy Lambda Function for DynamoDB
```bash
aws lambda create-function \
  --function-name cpp-dynamodb-demo \
  --role arn:aws:iam::<ACCOUNT_ID>:role/lambda-demo \
  --runtime provided.al2 \
  --timeout 15 \
  --memory-size 128 \
  --handler ddb-demo \
  --zip-file fileb://dynamo.zip
```

### Invoke Lambda with DynamoDB Payload
```bash
aws lambda invoke \
  --function-name cpp-dynamodb-demo \
  --payload '{
    "pathParameters": {
      "productId": "B01ETPUQ6E"
    },
    "queryStringParameters": {
      "startDate": "2024-01-01T00:00:00Z",
      "endDate": "2024-02-22T23:59:59Z"
    }
  }' \
  --cli-binary-format raw-in-base64-out \
  response.json
```

### Invoke via API Gateway
```bash
curl -X GET \
  'https://<your-api-id>.execute-api.<region>.amazonaws.com/<stage>/products/B01ETPUQ6E?startDate=2024-01-01T00:00:00Z&endDate=2024-02-22T23:59:59Z'
```

## AWS Lambda with S3

### Grant S3 Access to Lambda
```bash
aws iam attach-role-policy \
  --role-name lambda-demo \
  --policy-arn arn:aws:iam::aws:policy/AmazonS3ReadOnlyAccess
```

### Deploy Lambda Function for S3
```bash
aws lambda create-function \
  --function-name encode-file \
  --role arn:aws:iam::<ACCOUNT_ID>:role/lambda-demo \
  --runtime provided.al2 \
  --timeout 15 \
  --memory-size 128 \
  --handler encoder \
  --zip-file fileb://encoder.zip
```

### Invoke Lambda with S3 Payload
```bash
aws lambda invoke \
  --function-name encode-file \
  --payload '{
    "s3bucket": "my-test-bucket",
    "s3key": "path/to/my/file.txt"
  }' \
  --cli-binary-format raw-in-base64-out \
  response.json
```


