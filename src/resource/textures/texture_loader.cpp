#include "texture_loader.h"

#include <iostream>

#include <stb/stb_image.h>

TextureCPUData TextureLoader::load_from_file(const std::string& path) {
    TextureCPUData result;

    // stb_image returns pixels in row-major order (bottom-up/top-down
    // depends on stbi_set_flip_vertically_on_load).
    int width = 0;
    int height = 0;
    int channels_in_file = 0;

    // We request 4 channels (RGBA) to simplify GPU upload logic.
    constexpr int desired_channels = 4;

    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels_in_file, desired_channels);

    if (!data) {
        std::cerr << "TextureLoader::load_from_file: failed to load image '" << path << "'\n";
        return result;
    }

    result.width    = width;
    result.height   = height;
    result.channels = desired_channels;
    const std::size_t size = static_cast<std::size_t>(width) *
                             static_cast<std::size_t>(height) *
                             static_cast<std::size_t>(desired_channels);
    result.pixels.assign(data, data + size);

    stbi_image_free(data);
    return result;
}
