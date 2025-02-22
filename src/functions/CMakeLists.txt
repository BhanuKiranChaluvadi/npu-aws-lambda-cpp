# Add each Lambda function
add_subdirectory(create_creation)

# Common settings for all Lambda functions
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/functions)