# npu-aws-lambda-cpp

A C++ AWS Lambda project demonstrating AWS service integrations.

## Prerequisites

Before building and running this project, ensure that you have the following:

- A properly configured development environment
- [AWS SDK for C++](https://github.com/aws/aws-sdk-cpp)
- [AWS Lambda C++](https://github.com/awslabs/aws-lambda-cpp)

## Building and Compilation

### Using AWS Base Image for Lambda

AWS provides base images for Lambda, which can be found [here](https://docs.aws.amazon.com/lambda/latest/dg/images-create.html). However, there is no official base image for C++. Therefore, we will use a compatible base image as our development environment and manually install the necessary packages.

### Setting Up the Development Environment

To set up the development environment, you can pull the necessary Amazon Linux images:
```
docker pull public.ecr.aws/amazonlinux/amazonlinux:2
docker pull public.ecr.aws/amazonlinux/amazonlinux:2023
```

For compiling and installing `AWS SDK CPP` and `AWS Lambda CPP` , refer to the detailed instructions provided in the [AWS Lambda C++ Setup Guide](./docs/aws/AWS_Lambda_CPP_Setup_Guide.md).

### Project Architecture and API Reference

- **High-Level Software Architecture:** See [architecture.md](./docs/architecture.md) for an overview of the system design.
- **API Documentation:** Refer to [endpoints.md](./docs/api/endpoints.md) for details on available API endpoints.

## References

Here are some useful resources related to AWS Lambda and C++ development:

- [AWS Lambda Pricing Calculator](https://s3.amazonaws.com/lambda-tools/pricing-calculator.html)
- [AWS CLI Installation Guide](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html)
- [AWS SDK for C++](https://aws.amazon.com/sdk-for-cpp/)
- [AWS Code Examples for C++](https://github.com/awsdocs/aws-doc-sdk-examples/tree/main/cpp/example_code)
- [AWS Lambda C++ Runtime](https://github.com/awslabs/aws-lambda-cpp)
- [C++ Lambda Runtime Blog Post](https://aws.amazon.com/blogs/compute/introducing-the-c-lambda-runtime/)

---

