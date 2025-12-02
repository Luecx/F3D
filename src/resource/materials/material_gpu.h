#pragma once

#include <cstdint>

struct GPU_ColorComponent {
    int   enabled;      // 0 = constant, 1 = texture
    float color[3];     // RGB if constant
    std::uint64_t texture_handle; // bindless handle or 0
};

struct GPU_ScalarComponent {
    int   enabled;      // 0 = constant, 1 = texture
    float value;        // scalar if constant
    std::uint64_t texture_handle; // bindless handle or 0
};

struct GPU_TextureComponent {
    int   enabled;          // 0 or 1
    std::uint64_t texture_handle;
    int   padding1, padding2, padding3;
};

struct GPU_Material {
    GPU_ColorComponent base_color;
    GPU_ColorComponent emission;
    GPU_ColorComponent sheen_color;
    GPU_ColorComponent subsurface_color;
    GPU_ColorComponent subsurface_radius;

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
    float padding1;
    float padding2;

    GPU_TextureComponent normal_map;
    GPU_TextureComponent displacement_map;
    GPU_TextureComponent ambient_occlusion_map;
};
