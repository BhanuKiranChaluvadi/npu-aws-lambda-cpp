#include "s3_service.hpp"
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/core/utils/base64/Base64.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <sstream>

S3Service::S3Service(
    const Aws::S3::S3Client &client,
    std::string_view bucket) noexcept
    : client_(client), bucket_name_(bucket) {}

S3Service::UploadResult S3Service::upload_creation_image(
    std::string_view creation_id,
    std::string_view image_data) const
{

    // AWS_LOGSTREAM_INFO("S3Service", "Image data length: " << image_data.length());
    // AWS_LOGSTREAM_INFO("S3Service", "Image data: " << image_data);

    if (!validate_image_data(image_data))
    {
        throw std::invalid_argument("Invalid image data format");
    }

    try
    {
        // Generate keys for both image and thumbnail
        std::string image_key = std::string("images/") + std::string(creation_id) + ".jpg";
        std::string thumb_key = std::string("thumbnails/") + std::string(creation_id) + ".jpg";

        // Upload original image
        upload_image(image_key, image_data);

        // Create and upload thumbnail
        std::string thumbnail_data = create_thumbnail(image_data);
        upload_image(thumb_key, thumbnail_data);

        return UploadResult{
            .image_key = std::move(image_key),
            .thumbnail_key = std::move(thumb_key)};
    }
    catch (const std::exception &e)
    {
        AWS_LOGSTREAM_ERROR("S3Service",
                            "Failed to upload creation image: " << e.what());
        throw;
    }
}

std::string S3Service::upload_image(
    std::string_view key,
    std::string_view image_data) const
{
    AWS_LOGSTREAM_INFO("S3Service", "Uploading image with key: " << key);

    if (key.empty() || image_data.empty())
    {
        throw std::invalid_argument("Empty key or image data");
    }

    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucket_name_);
    request.SetKey(std::string(key));
    request.SetContentType("image/jpeg");

    // Convert base64 to binary
    Aws::Utils::Base64::Base64 base64;
    
    // Remove potential "data:image/jpeg;base64," prefix if present
    std::string base64Data = std::string(image_data);
    size_t pos = base64Data.find(",");
    if (pos != std::string::npos) {
        base64Data = base64Data.substr(pos + 1);
    }

    // Decode base64 data
    Aws::Utils::ByteBuffer decoded = base64.Decode(base64Data);

    if (decoded.GetLength() == 0)
    {
        throw std::runtime_error("Failed to decode base64 image data");
    }

    AWS_LOGSTREAM_INFO("S3Service", "Decoded image size: " << decoded.GetLength() << " bytes");

    // Create input stream from decoded binary data
    auto input_data = Aws::MakeShared<Aws::StringStream>("ImageData");
    input_data->write(reinterpret_cast<const char*>(decoded.GetUnderlyingData()),
                     static_cast<std::streamsize>(decoded.GetLength()));

    request.SetBody(input_data);
    request.SetContentLength(static_cast<long>(decoded.GetLength()));

    // Upload to S3
    auto outcome = client_.PutObject(request);
    if (!outcome.IsSuccess())
    {
        throw std::runtime_error("Failed to upload image: " +
                               outcome.GetError().GetMessage());
    }

    AWS_LOGSTREAM_INFO("S3Service", "Successfully uploaded image to: " << key);
    return std::string(key);
}

std::string S3Service::create_thumbnail(std::string_view image_data) const
{
    // TODO: Implement actual image resizing
    // For now, return the original image
    return std::string(image_data);
}

bool S3Service::validate_image_data(std::string_view image_data) const noexcept
{
    if (image_data.empty())
    {
        return false;
    }

    // Convert base64 to binary
    Aws::Utils::Base64::Base64 base64;
    Aws::Utils::ByteBuffer decoded = base64.Decode(std::string(image_data));

    // Check if decoding was successful
    return decoded.GetLength() > 0;
}

void S3Service::delete_images(
    std::string_view image_key,
    std::string_view thumbnail_key) const
{

    // Delete main image
    Aws::S3::Model::DeleteObjectRequest request;
    request.SetBucket(bucket_name_);
    request.SetKey(std::string(image_key));

    auto mainOutcome = client_.DeleteObject(request);
    if (!mainOutcome.IsSuccess())
    {
        throw std::runtime_error("Failed to delete main image: " +
                                 mainOutcome.GetError().GetMessage());
    }

    // Delete thumbnail
    request.SetKey(std::string(thumbnail_key));
    auto thumbOutcome = client_.DeleteObject(request);
    if (!thumbOutcome.IsSuccess())
    {
        throw std::runtime_error("Failed to delete thumbnail: " +
                                 thumbOutcome.GetError().GetMessage());
    }
}