#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace resources {

//! Ordered storage states a resource can occupy.
enum class ResourceState : std::uint8_t { Drive = 0, Ram = 1, Gpu = 2 };

inline constexpr std::size_t kResourceStateCount = 3;

inline constexpr std::size_t to_index(ResourceState state) { return static_cast<std::size_t>(state); }

inline constexpr ResourceState next_state(ResourceState state) {
    switch (state) {
    case ResourceState::Drive:
        return ResourceState::Ram;
    case ResourceState::Ram:
        return ResourceState::Gpu;
    case ResourceState::Gpu:
        return ResourceState::Gpu;
    }
    return ResourceState::Drive;
}

inline constexpr ResourceState previous_state(ResourceState state) {
    switch (state) {
    case ResourceState::Drive:
        return ResourceState::Drive;
    case ResourceState::Ram:
        return ResourceState::Drive;
    case ResourceState::Gpu:
        return ResourceState::Ram;
    }
    return ResourceState::Drive;
}

inline constexpr std::string_view to_string(ResourceState state) {
    switch (state) {
    case ResourceState::Drive:
        return "DRIVE";
    case ResourceState::Ram:
        return "RAM";
    case ResourceState::Gpu:
        return "GPU";
    }
    return "UNKNOWN";
}

enum class ResourceOpType : std::uint8_t { Load, Unload };

} // namespace resources
