# npu-aws-lambda-cpp

- [c-lambda-runtime](https://aws.amazon.com/blogs/compute/introducing-the-c-lambda-runtime/)
- [pricing calculator](https://s3.amazonaws.com/lambda-tools/pricing-calculator.html)
- [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html)
-[aws-lambda-cpp](https://github.com/awslabs/aws-lambda-cpp)



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

```
docker pull amazonlinux:2

docker run -it amazonlinux:latest /bin/bash

yum update -y
yum install -y nano curl wget unzip tar

yum install -y cmake git make zip libcurl-devel

yum groupinstall -y "Development Tools"
yum install -y gcc gcc-c++

```

```
bash-5.2# pwd
/aws-lambda-cpp/build

/root/lambda-install
```