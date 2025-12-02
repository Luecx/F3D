#pragma once

#include <cstdint>
#include <vector>

/**
 * @brief CPU-side representation of texture pixel data.
 *
 * Pure container: no GPU/OpenGL concepts.
 */
struct TextureCPUData {
    int width    = 0;
    int height   = 0;
    int channels = 0;                  // 1, 3, 4 typical
    std::vector<std::uint8_t> pixels;  // tightly packed

    [[nodiscard]] bool valid() const noexcept {
        return width > 0 && height > 0 && channels > 0 && !pixels.empty();
    }

    void reset() noexcept {
        width = 0;
        height = 0;
        channels = 0;
        pixels.clear();
        pixels.shrink_to_fit();
    }
};
