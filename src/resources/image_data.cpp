//
// Created by Luecx on 23.06.2024.
//

#include "image_data.h"

bool ImageData::load_cpu_specific() {
    // Image-specific CPU loading logic here
    return true;
}

bool ImageData::load_gpu_specific() {
    // Image-specific GPU loading logic here
    return true;
}

bool ImageData::unload_cpu_specific() {
    // Image-specific CPU unloading logic here
    return true;
}

bool ImageData::unload_gpu_specific() {
    // Image-specific GPU unloading logic here
    return true;
}

