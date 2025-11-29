#pragma once

#include <cstdint>
#include <string_view>

namespace resources {

/**
 * @brief Basic storage state flags for resource data.
 *
 * These values indicate *where* the data is currently available.
 *
 * Important: In the new design a resource can be in multiple states at once.
 * For example:
 *   - A texture might be loaded on RAM *and* uploaded to the GPU.
 *   - Or it might be only on the GPU (RAM copy freed).
 *
 * This is different from a single "current_state" model. Instead you typically
 * track boolean flags (e.g. `has_ram`, `has_gpu`) and use these enum values
 * as targets for request/release operations.
 */
enum class ResourceState : std::uint8_t {
    Drive = 0, ///< Data exists only on disk / drive (baseline existence).
    Ram   = 1, ///< Data is loaded in main memory (CPU accessible).
    Gpu   = 2  ///< Data is uploaded to GPU memory.
};

/**
 * @brief Human-readable names for @ref ResourceState values.
 */
inline constexpr std::string_view to_string(ResourceState state) {
    switch (state) {
    case ResourceState::Drive: return "DRIVE";
    case ResourceState::Ram:   return "RAM";
    case ResourceState::Gpu:   return "GPU";
    }
    return "UNKNOWN";
}

/**
 * @brief Operation type for the loading thread / resource queues.
 *
 * These operations are interpreted as:
 *   - Load(Drive) : ensure that the on-disk file exists / is tracked.
 *   - Load(Ram)   : load from disk into CPU memory.
 *   - Load(Gpu)   : upload from RAM (or Drive) to GPU memory.
 *
 *   - Unload(Ram) : free CPU-side memory, keep Drive/Gpu as requested.
 *   - Unload(Gpu) : free GPU memory, keep Drive/Ram as requested.
 *
 * How exactly these are implemented is up to the resource class.
 */
enum class ResourceOpType : std::uint8_t {
    Load,   ///< Request that a given state becomes available.
    Unload  ///< Request that a given state may be released.
};

} // namespace resources
