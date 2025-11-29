#pragma once

#include "resource_data.h"
#include "image_buffer.h"
#include "../gldata/texture_data.h"

// Forward declaration of ImageBuffer class
class ImageBuffer;

struct ImageData : public ResourceData {
    explicit ImageData(std::string path);

    // Image properties
    int width{0}, height{0}, channels{0};

    // CPU image data (managed through ImageBuffer class)
    ImageBuffer::UPtr cpu_data;

    // GPU image data (OpenGL texture wrapper)
    TextureData::UPtr gpu_data;

    // CPU and GPU load/unload overrides
    bool load_to_ram() override;
    void unload_from_ram() override;
    bool load_to_gpu() override;
    void unload_from_gpu() override;
};
