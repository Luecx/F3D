#include "texture_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "../../logging/logging.h"
#include "../resource_logging.h"

TextureCPUData TextureLoader::load_from_file(const std::string& path) {
    TextureCPUData result;

    int width = 0;
    int height = 0;
    int channels_in_file = 0;

    constexpr int desired_channels = 4; // request RGBA for simplicity

    stbi_set_flip_vertically_on_load(false);
    unsigned char* data =
        stbi_load(path.c_str(), &width, &height, &channels_in_file, desired_channels);

    if (!data) {
        logging::log(reslog::TEXTURE, logging::ERROR,
                     "TextureLoader::load_from_file: failed to load image '" + path + "'");
        return result;
    }

    result.width    = width;
    result.height   = height;
    result.channels = desired_channels;

    const std::size_t size =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height) *
        static_cast<std::size_t>(desired_channels);

    result.pixels.assign(data, data + size);

    stbi_image_free(data);
    return result;
}
