#pragma once
#include <aws/s3/S3Client.h>
#include <string_view>
#include "../models/creation.hpp"

class S3Service {
public:
    /**
     * @brief Construct a new S3Service
     * @param client Reference to AWS S3 client
     * @param bucket Name of the S3 bucket
     */
    explicit S3Service(
        const Aws::S3::S3Client& client, 
        std::string_view bucket) noexcept;

    /**
     * @brief Result structure for image upload operations
     */
    struct UploadResult {
        std::string image_key;
        std::string thumbnail_key;
    };

    /**
     * @brief Upload an image and its thumbnail for a creation
     * @param creation_id Unique identifier for the creation
     * @param image_data Base64 encoded image data
     * @throws std::runtime_error if upload fails
     * @throws std::invalid_argument if validation fails
     * @return UploadResult containing the keys of uploaded files
     */
    UploadResult upload_creation_image(
        std::string_view creation_id,
        std::string_view image_data) const;

    /**
     * @brief Delete both main image and thumbnail
     * @param image_key Key of the main image
     * @param thumbnail_key Key of the thumbnail
     * @throws std::runtime_error if deletion fails
     */
    void delete_images(
        std::string_view image_key,
        std::string_view thumbnail_key) const;

private:
    /**
     * @brief Create a thumbnail from the original image
     * @param image_data Base64 encoded image data
     * @return Base64 encoded thumbnail data
     */
    std::string create_thumbnail(std::string_view image_data) const;

    /**
     * @brief Upload a single image to S3
     * @param key S3 object key
     * @param image_data Base64 encoded image data
     * @return S3 object key of uploaded file
     */
    std::string upload_image(
        std::string_view key,
        std::string_view image_data) const;

    /**
     * @brief Validate the format of image data
     * @param image_data Base64 encoded image data
     * @return true if valid, false otherwise
     */
    bool validate_image_data(std::string_view image_data) const noexcept;

    const Aws::S3::S3Client& client_;
    const std::string bucket_name_;
};