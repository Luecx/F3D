#pragma once

#include <string>

#include "texture_cpu_data.h"

/**
 * @brief Helper class for loading image files into @ref TextureCPUData.
 *
 * This class is responsible for decoding image files from disk into
 * raw pixel buffers using stb_image (or another image library).
 */
class TextureLoader {
public:
    /**
     * @brief Loads image data from the given file path into CPU memory.
     *
     * The returned @ref TextureCPUData will contain pixel data in either
     * 1, 3, or 4 channels depending on the original file. By default,
     * the loader will request 4 channels (RGBA) for simplicity.
     *
     * On failure, the returned TextureCPUData will have width/height 0
     * and an empty pixel buffer.
     *
     * @param path Filesystem path to the image file.
     * @return Loaded CPU texture data.
     */
    static TextureCPUData load_from_file(const std::string& path);
};
