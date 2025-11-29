//
// Created by Luecx on 23.06.2024.
//

#include "image_data.h"

#include "image_buffer.h"

#include <iostream>

ImageData::ImageData(std::string path) : ResourceData(std::move(path)) { set_label("Image"); }

bool ImageData::load_to_ram() {
    cpu_data = std::make_unique<ImageBuffer>(get_path());
    if (!cpu_data) {
        return false;
    }
    width = cpu_data->width();
    height = cpu_data->height();
    channels = cpu_data->channels();
    return true;
}

bool ImageData::load_to_gpu() {
    if (!cpu_data) {
        std::cerr << "No CPU data loaded for image: " << get_path() << std::endl;
        return false;
    }

    gpu_data = std::make_unique<TextureData>(TextureType::TEX_2D);

    GLenum format = GL_RGB;
    GLenum internal_format = GL_RGB8;
    if (channels == 1) {
        format = GL_RED;
        internal_format = GL_R8;
    } else if (channels == 3) {
        format = GL_RGB;
        internal_format = GL_RGB8;
    } else if (channels == 4) {
        format = GL_RGBA;
        internal_format = GL_RGBA8;
    }

    TextureSpecification spec;
    spec.type = TextureType::TEX_2D;
    spec.internal_format = internal_format;
    spec.data_format = format;
    spec.data_type = GL_UNSIGNED_BYTE;
    spec.wrap_s = spec.wrap_t = GL_REPEAT;
    spec.wrap_r = GL_REPEAT;

    const void* data_array[6] = {cpu_data->data(), nullptr, nullptr, nullptr, nullptr, nullptr};
    gpu_data->set_data(width, height, spec, data_array);

    return true;
}

void ImageData::unload_from_ram() {
    cpu_data.reset();
    width = height = channels = 0;
}

void ImageData::unload_from_gpu() { gpu_data.reset(); }
