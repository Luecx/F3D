#pragma once

#include <memory>

#include "../../gldata/texture_data.h"
#include "texture_cpu_data.h"

/**
 * @brief GPU-side representation of a texture.
 *
 * Wraps a TextureData (OpenGL texture) and knows how to upload CPU data.
 * Does NOT manage refcounts or lifetimes; Texture (resource) drives it.
 */
class TextureGPUData {
    public:
    using UPtr = std::unique_ptr<TextureGPUData>;

    TextureGPUData()  = default;
    ~TextureGPUData() = default;

    TextureGPUData(const TextureGPUData&)            = delete;
    TextureGPUData& operator=(const TextureGPUData&) = delete;

    TextureGPUData(TextureGPUData&&) noexcept            = default;
    TextureGPUData& operator=(TextureGPUData&&) noexcept = default;

    [[nodiscard]] bool is_valid() const noexcept { return static_cast<bool>(texture_); }

    /// Upload the given CPU data into a GPU texture (creates/replaces TextureData).
    void upload_from_cpu(const TextureCPUData& cpu);

    /// Destroy the underlying GPU texture object (if any).
    void release();

    [[nodiscard]] TextureData*       texture()       noexcept { return texture_.get(); }
    [[nodiscard]] const TextureData* texture() const noexcept { return texture_.get(); }

    /// Bindless handle helper (0 if not available).
    [[nodiscard]] GLuint64 get_handle() const {
        return texture_ ? texture_->get_handle() : 0;
    }

    private:
    TextureData::UPtr texture_;
};
