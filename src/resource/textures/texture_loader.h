#pragma once

#include <string>

#include "texture_cpu_data.h"

/**
 * @brief Helper for loading image files into TextureCPUData.
 *
 * Uses stb_image (or similar) under the hood.
 */
class TextureLoader {
    public:
    /**
     * @brief Loads image data from the given file path into CPU memory.
     *
     * On failure, returns an invalid TextureCPUData (width/height 0, empty pixels).
     */
    static TextureCPUData load_from_file(const std::string& path);
};
