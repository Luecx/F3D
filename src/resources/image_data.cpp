//
// Created by Luecx on 23.06.2024.
//

#include "image_data.h"

bool ImageData::load_cpu_specific() {
    // Image-specific CPU loading logic here
    return true;
}

bool ImageData::load_gpu_specific() {
    gpu_data = std::make_unique<TextureData>();
    return true;
}

bool ImageData::unload_cpu_specific() {
    delete cpu_data.release();
    return true;
}

bool ImageData::unload_gpu_specific() {
    gpu_data = nullptr;
    return true;
}

