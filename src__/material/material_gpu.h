#pragma once

#include <cstdint>

// CPU copy of GPU-side structs for material data.
// These definitions should match your GLSL definitions (with std430 alignment considerations).

struct GPU_ColorComponent {
    int enabled;             // 0 = constant; 1 = texture
    float color[3];          // Constant color value
    uint64_t texture_handle; // Bindless texture handle (0 if unused)
};

struct GPU_ScalarComponent {
    int enabled;             // 0 = constant; 1 = texture
    float value;             // Constant scalar value
    uint64_t texture_handle; // Bindless texture handle
};

struct GPU_TextureComponent {
    int enabled;                      // 0 = not available; 1 = available
    uint64_t texture_handle;          // Bindless texture handle
    int padding1, padding2, padding3; // Padding for std430 alignment
};

struct GPU_Material {
    GPU_ColorComponent base_color;
    GPU_ColorComponent emission;
    GPU_ColorComponent sheen_color; // For example; you can adjust as needed

    GPU_ScalarComponent roughness;
    GPU_ScalarComponent metallic;
    GPU_ScalarComponent specular;
    GPU_ScalarComponent specular_tint;
    GPU_ScalarComponent transmission;
    GPU_ScalarComponent transmission_roughness;
    GPU_ScalarComponent clearcoat;
    GPU_ScalarComponent clearcoat_roughness;
    GPU_ScalarComponent subsurface;
    GPU_ScalarComponent sheen;
    GPU_ScalarComponent sheen_tint;
    GPU_ScalarComponent anisotropy;

    float ior;
    float anisotropy_rotation;
    float padding1, padding2;

    GPU_TextureComponent normal_map;
    GPU_TextureComponent tangent_map;
};
