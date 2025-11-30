#pragma once

#include <memory>

#include "../../gldata/texture_data.h"
#include "texture_cpu_data.h"

/**
 * @brief GPU-side representation of a texture.
 *
 * This class owns an underlying TextureData object, which wraps
 * an OpenGL texture. It provides a minimal interface for uploading
 * CPU-side pixels and releasing the GPU resource again.
 *
 * It does *not* handle async or job queues; that is the responsibility
 * of the higher-level Texture / TextureManager classes.
 */
class TextureGPUData {
  public:
    using UPtr = std::unique_ptr<TextureGPUData>;

    TextureGPUData() = default;
    ~TextureGPUData() = default;

    TextureGPUData(const TextureGPUData&) = delete;
    TextureGPUData& operator=(const TextureGPUData&) = delete;

    TextureGPUData(TextureGPUData&&) noexcept = default;
    TextureGPUData& operator=(TextureGPUData&&) noexcept = default;

    /**
     * @brief Returns true if a GPU texture object has been created.
     */
    [[nodiscard]] bool is_valid() const noexcept { return static_cast<bool>(texture_); }

    /**
     * @brief Uploads the given CPU pixel data to the GPU, creating
     *        or replacing the underlying TextureData.
     *
     * This function assumes it is called on a thread that owns a valid
     * OpenGL context (usually the main/render thread).
     *
     * @param cpu CPU-side texture data. Must be valid().
     */
    void upload_from_cpu(const TextureCPUData& cpu);

    /**
     * @brief Releases the underlying GPU texture, if any.
     *
     * After calling this, @ref is_valid() returns false, and the OpenGL
     * texture handle is destroyed.
     */
    void release();

    /**
     * @brief Returns the underlying TextureData, or nullptr if not valid.
     */
    [[nodiscard]] TextureData* texture() noexcept { return texture_.get(); }

    /**
     * @brief Returns the underlying TextureData, or nullptr if not valid.
     */
    [[nodiscard]] const TextureData* texture() const noexcept { return texture_.get(); }

    /**
     * @brief Returns bindless handle (or 0 if not available).
     */
    [[nodiscard]] GLuint64 get_handle() const {
        if (texture_) {
            return texture_->get_handle();
        }
        return 0;
    }

  private:
    TextureData::UPtr texture_;
};
