## Pull Docker
```bash
docker pull amazonlinux:latest

docker run -it amazonlinux:latest /bin/bash

yum update -y
yum install -y nano wget unzip tar
yum install -y openssl-devel
yum install -y cmake git make zip libcurl-devel
yum groupinstall -y "Development Tools"
yum install -y gcc gcc-c++
yum install -y tree
yum install -y zlib-devel
```

## Compile AWS-SDK-CPP 
```bash
mkdir ~/install
git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
cd aws-sdk-cpp
mkdir build
cd build
cmake .. -DBUILD_ONLY="core;dynamodb;s3" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCUSTOM_MEMORY_MANAGEMENT=OFF \
    -DCMAKE_INSTALL_PREFIX=~/install

make
make install
```

## Compile AWS-LAMBDA-CPP
```bash
git clone https://github.com/awslabs/aws-lambda-cpp.git
cd aws-lambda-cpp
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=~/install \
make
make install
```

## Examples

### CMAKE patch
Modify CMAKE in examples if the compilation fails becuase of ZLIB error
```bash
find_package(ZLIB REQUIRED)  # before core/s3/dynamodb

find_package(AWSSDK COMPONENTS core) 

target_link_libraries(${PROJECT_NAME} PUBLIC 
    AWS::aws-lambda-runtime 
    ${AWSSDK_LINK_LIBRARIES}
    ZLIB::ZLIB
)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    AWS::aws-lambda-runtime 
    ${AWSSDK_LINK_LIBRARIES}
    ZLIB::ZLIB  # add ZLIB lib
)
```

### demo

```bash
aws lambda create-function --function-name demo \
    --role arn:aws:iam::242201308302:role/lambda-demo \
    --runtime provided.al2 \
    --timeout 15 \
    --memory-size 128 \
    --handler demo \
    --zip-file fileb://hello.zip

aws lambda invoke \
    --function-name demo \
    --payload fileb://payload.json \
    output.txt
```

### api-gateway
```bash
aws lambda create-function \
    --function-name cpp-api-gateway-demo \
    --role arn:aws:iam::242201308302:role/lambda-demo \
    --runtime provided.al2 \
    --timeout 15 \
    --memory-size 128 \
    --handler api \
    --zip-file fileb://api.zip

curl -v -X POST \
    'https://vbp29pnl14.execute-api.eu-north-1.amazonaws.com/default/cpp-api-gateway-demo?name=Bradley&city=Chicago' \
    -H 'content-type: application/json' \
    -H 'day: Sunday' \
    -d '{ "time": "evening" }'
```

### dynamodb
```bash
aws lambda create-function \
    --function-name cpp-dynamodb-demo \
    --role arn:aws:iam::242201308302:role/lambda-demo \
    --runtime provided.al2 \
    --timeout 15 \
    --memory-size 128 \
    --handler dynamo \
    --zip-file fileb://dynamo.zip

aws lambda invoke \
  --function-name cpp-dynamodb-demo \
  --payload '{
    "pathParameters": {
      "productId": "B01234ABCD"
    },
    "queryStringParameters": {
      "startDate": "2024-01-01T00:00:00Z",
      "endDate": "2024-02-21T23:59:59Z"
    }
  }' \
  --cli-binary-format raw-in-base64-out \
  response.json

```

### s3
```bash
aws lambda create-function \
  --function-name cpp-s3-demo \
  --role arn:aws:iam::242201308302:role/lambda-demo \
  --runtime provided.al2 \
  --timeout 15 \
  --memory-size 128 \
  --handler s3 \
  --zip-file fileb://encoder.zip

aws lambda invoke \
  --function-name cpp-s3-demo \
  --payload '{
    "s3bucket": "my-test-bucket",
    "s3key": "path/to/my/file.txt"
  }' \
  --cli-binary-format raw-in-base64-out \
  response.json
```


## Useful paths inside the docker
```bash
bash-5.2# pwd
/aws-sdk-cpp/build
/root/install
```



