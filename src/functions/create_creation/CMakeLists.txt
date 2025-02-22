project(create_creation LANGUAGES CXX)

# Create executable
add_executable(${PROJECT_NAME} 
    main.cpp
    creation_handler.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ~/install/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        npu_common_lib
        AWS::aws-lambda-runtime 
        ${AWSSDK_LINK_LIBRARIES}
        ZLIB::ZLIB
)

# Compiler options
target_compile_options(${PROJECT_NAME} 
    PRIVATE
        -Wall
        -Wextra
        -static
)

# Package Lambda function
aws_lambda_package_target(${PROJECT_NAME})

# Create deployment package
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E tar "cfv" 
#         "${CMAKE_BINARY_DIR}/bin/functions/${PROJECT_NAME}/${PROJECT_NAME}.zip"
#         --format=zip
#         "${CMAKE_BINARY_DIR}/bin/functions/${PROJECT_NAME}/${PROJECT_NAME}"
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     COMMENT "Creating deployment package for ${PROJECT_NAME}"
# )