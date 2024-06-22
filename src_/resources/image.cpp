#include "image.h"
#include <iostream>
#include <stb/stb_image.h>

ImageResource::ImageResource(ResourceManager* manager, const std::string& name)
    : Resource(manager, name) {}

void ImageResource::do_load() {
    // Simulate loading image data
    std::cout << "Loading image: " << get_name() << std::endl;
    loaded_ = true;
}

void ImageResource::do_unload() {
    // Simulate unloading image data


    loaded_ = false;
}