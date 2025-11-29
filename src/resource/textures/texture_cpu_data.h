#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief CPU-side representation of texture pixel data.
 *
 * This struct holds raw pixel data decoded from an image file
 * (e.g. PNG, JPG, HDR) and basic metadata such as width, height,
 * and number of channels.
 *
 * It does *not* know anything about GPU objects or OpenGL; it is
 * purely a CPU-side container that can be filled by a loader
 * (e.g. TextureLoader) and later consumed by TextureGPUData.
 */
struct TextureCPUData {
    /// Width of the texture in pixels.
    int width = 0;

    /// Height of the texture in pixels.
    int height = 0;

    /**
     * @brief Number of channels per pixel.
     *
     * Typical values:
     * - 1: R
     * - 3: RGB
     * - 4: RGBA
     */
    int channels = 0;

    /**
     * @brief Raw pixel data in row-major order.
     *
     * The exact layout (e.g. RGBA vs. RGB) is implied by @ref channels.
     */
    std::vector<std::uint8_t> pixels;

    /**
     * @brief Returns true if this object holds valid, non-empty pixel data.
     */
    [[nodiscard]] bool valid() const noexcept {
        return width > 0 && height > 0 && channels > 0 && !pixels.empty();
    }

    /**
     * @brief Clears all pixel data and resets dimensions.
     */
    void reset() noexcept {
        width = 0;
        height = 0;
        channels = 0;
        pixels.clear();
        pixels.shrink_to_fit();
    }
};
