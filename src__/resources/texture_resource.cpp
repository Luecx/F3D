#include "texture_resource.h"

#include <iostream>

TextureResource::TextureResource(const std::string& key, std::shared_ptr<ImageData> image,
                                 TextureSpecification specification)
    : ResourceData(key), image_(std::move(image)), spec_(specification) {
    set_label("Texture");
    if (image_) {
        register_dependency(resources::ResourceState::Ram, image_, resources::ResourceState::Ram);
        register_dependency(resources::ResourceState::Gpu, image_, resources::ResourceState::Ram);
    }
}

bool TextureResource::load_to_ram() { return static_cast<bool>(image_); }

void TextureResource::unload_from_ram() {}

bool TextureResource::load_to_gpu() {
    if (!image_ || !image_->cpu_data) {
        std::cerr << "TextureResource missing CPU data for " << get_path() << std::endl;
        return false;
    }

    gpu_texture_ = std::make_unique<TextureData>(spec_.type);

    TextureSpecification upload_spec = spec_;
    if (image_->channels == 1) {
        upload_spec.data_format = GL_RED;
        upload_spec.internal_format = GL_R8;
    } else if (image_->channels == 3) {
        upload_spec.data_format = GL_RGB;
        upload_spec.internal_format = GL_RGB8;
    } else if (image_->channels == 4) {
        upload_spec.data_format = GL_RGBA;
        upload_spec.internal_format = GL_RGBA8;
    }

    const void* planes[6] = {image_->cpu_data->data(), nullptr, nullptr, nullptr, nullptr, nullptr};
    gpu_texture_->set_data(image_->width, image_->height, upload_spec, planes);
    return true;
}

void TextureResource::unload_from_gpu() { gpu_texture_.reset(); }
