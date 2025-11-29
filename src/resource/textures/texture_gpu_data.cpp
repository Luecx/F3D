#include "texture_gpu_data.h"

#include <stdexcept>

/**
 * @brief Uploads CPU data to the GPU texture.
 */
void TextureGPUData::upload_from_cpu(const TextureCPUData& cpu) {
    if (!cpu.valid()) {
        throw std::runtime_error("TextureGPUData::upload_from_cpu: CPU data is invalid");
    }

    // Lazily create the TextureData if needed.
    if (!texture_) {
        texture_ = std::make_unique<TextureData>(TextureType::TEX_2D);
    }

    TextureSpecification spec{};
    spec.type = TextureType::TEX_2D;

    // Choose a basic format based on channel count.
    if (cpu.channels == 1) {
        spec.internal_format = GL_R8;
        spec.data_format     = GL_RED;
    } else if (cpu.channels == 3) {
        spec.internal_format = GL_RGB8;
        spec.data_format     = GL_RGB;
    } else {
        // Fallback to RGBA for 2 or 4 channels.
        spec.internal_format = GL_RGBA8;
        spec.data_format     = GL_RGBA;
    }

    spec.data_type          = GL_UNSIGNED_BYTE;
    spec.wrap_s             = GL_REPEAT;
    spec.wrap_t             = GL_REPEAT;
    spec.wrap_r             = GL_REPEAT;
    spec.min_filter         = GL_LINEAR_MIPMAP_LINEAR;
    spec.mag_filter         = GL_LINEAR;
    spec.generate_mipmaps   = true;

    const void* planes[6] = {
        cpu.pixels.data(), nullptr, nullptr, nullptr, nullptr, nullptr};

    texture_->set_data(cpu.width, cpu.height, spec, planes);
}

/**
 * @brief Destroys the underlying GPU texture object.
 */
void TextureGPUData::release() {
    texture_.reset(); // TextureData destructor handles GL deletion.
}
