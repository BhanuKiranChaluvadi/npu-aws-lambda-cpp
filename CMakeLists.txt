cmake_minimum_required(VERSION 3.9)
project(npu-lambda)
set(CMAKE_CXX_STANDARD 17)

# Add custom install directory
list(APPEND CMAKE_PREFIX_PATH "~/install")

# Find required packages
find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3 dynamodb)

# Add source directory
add_subdirectory(src)