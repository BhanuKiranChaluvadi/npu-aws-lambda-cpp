# Create common library
add_library(npu_common_lib
    models/creation.cpp
    services/s3_service.cpp
    services/dynamodb_service.cpp
)

# Set include directories
target_include_directories(npu_common_lib 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ~/install/include
)

# Link AWS SDK libraries
target_link_libraries(npu_common_lib 
    PUBLIC
        ${AWSSDK_LINK_LIBRARIES}
        ZLIB::ZLIB
        # aws-cpp-sdk-core
        # aws-cpp-sdk-s3
        # aws-cpp-sdk-dynamodb
)