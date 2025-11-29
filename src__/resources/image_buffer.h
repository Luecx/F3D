#pragma once

#include <string>
#include <stdexcept>
#include <memory>

/**
 * @brief A simple class for loading and managing image data on the CPU.
 *
 * This class uses stb_image (included only in the source file) to load image files.
 * It allocates memory for the image data on the CPU and frees it on destruction.
 */
class ImageBuffer {
  public:
    using SPtr = std::shared_ptr<ImageBuffer>; ///< Shared pointer type alias.
    using UPtr = std::unique_ptr<ImageBuffer>; ///< Unique pointer type alias.

    /**
     * @brief Constructs an ImageBuffer by loading an image file.
     *
     * This constructor loads an image from the given filename.
     * If loading fails, it throws a std::runtime_error.
     *
     * @param filename The path to the image file to load.
     *
     * @exception std::runtime_error Thrown if the image file cannot be loaded.
     */
    explicit ImageBuffer(const std::string& filename);
    ImageBuffer() = default;

    /**
     * @brief Destructor that frees the allocated image data.
     */
    ~ImageBuffer();

    // Disable copy constructor and assignment operator to avoid double deletion.
    ImageBuffer(const ImageBuffer&) = delete;
    ImageBuffer& operator=(const ImageBuffer&) = delete;

    /**
     * @brief Move constructor that transfers ownership of image data.
     *
     * After the move, the other ImageBuffer is left in a safe, destructible state.
     *
     * @param other An rvalue reference to another ImageBuffer.
     */
    ImageBuffer(ImageBuffer&& other) noexcept;

    /**
     * @brief Move assignment operator that transfers ownership of image data.
     *
     * @param other An rvalue reference to another ImageBuffer.
     * @return A reference to this ImageBuffer.
     */
    ImageBuffer& operator=(ImageBuffer&& other) noexcept;

    /**
     * @brief Returns a pointer to the loaded image data.
     *
     * The data is stored in row-major order with each pixel consisting of _channels components.
     *
     * @return Pointer to the image data.
     */
    unsigned char* data() const;

    /**
     * @brief Gets the width of the image.
     *
     * @return The image width in pixels.
     */
    int width() const;

    /**
     * @brief Gets the height of the image.
     *
     * @return The image height in pixels.
     */
    int height() const;

    /**
     * @brief Gets the number of color channels in the image.
     *
     * @return The number of channels per pixel.
     */
    int channels() const;

  private:
    unsigned char* _data; ///< Pointer to the image data allocated on the CPU.
    int _width;           ///< Image width in pixels.
    int _height;          ///< Image height in pixels.
    int _channels;        ///< Number of color channels per pixel.
};
