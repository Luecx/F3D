#pragma once

#include <string_view>

enum class ResourceState : unsigned char {
    Drive = 0,  // conceptual: asset exists on disk; we usually don't manage this explicitly
    Cpu   = 1,  // in-RAM data
    Gpu   = 2   // GPU-resident data
};

inline constexpr std::string_view to_string(ResourceState state) {
    switch (state) {
        case ResourceState::Drive: return "DRIVE";
        case ResourceState::Cpu:   return "CPU";
        case ResourceState::Gpu:   return "GPU";
    }
    return "UNKNOWN";
}
