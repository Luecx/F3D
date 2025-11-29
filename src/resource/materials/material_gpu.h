#pragma once

#include <cstdint>

/**
 * @brief CPU-side mirror of the GPU color component for materials.
 *
 * This must match the GLSL definition in mat.glsl:
 *
 * struct GPU_ColorComponent {
 *     bool enabled;
 *     vec3 color;
 *     uint64_t texture_handle;
 * };
 *
 * On the C++ side we explicitly use a 32-bit integer for @ref enabled,
 * because GLSL bool in std430 also occupies 4 bytes and this gives us
 * precise control over the layout.
 */
struct GPU_ColorComponent {
    std::int32_t  enabled;         ///< 0 = constant color, 1 = sample texture
    float         color[3];        ///< Base RGB color (also default for textured case)
    std::uint64_t texture_handle;  ///< Bindless texture handle (0 = no texture)
};

/**
 * @brief CPU-side mirror of the GPU scalar component.
 *
 * GLSL:
 * struct GPU_ScalarComponent {
 *     bool enabled;
 *     float value;
 *     uint64_t texture_handle;
 * };
 */
struct GPU_ScalarComponent {
    std::int32_t  enabled;         ///< 0 = constant scalar, 1 = sample texture
    float         value;           ///< Constant scalar value or default for texture
    std::uint64_t texture_handle;  ///< Bindless texture handle (0 = no texture)
};

/**
 * @brief CPU-side mirror of the GPU texture component.
 *
 * GLSL:
 * struct GPU_TextureComponent {
 *     bool enabled;
 *     uint64_t texture_handle;
 *     int padding1;
 *     int padding2;
 *     int padding3;
 * };
 */
struct GPU_TextureComponent {
    std::int32_t  enabled;         ///< 0 = disabled, 1 = use texture
    std::uint64_t texture_handle;  ///< Bindless texture handle (0 = no texture)
    std::int32_t  padding1;        ///< Padding to match GLSL std430 layout
    std::int32_t  padding2;        ///< Padding to match GLSL std430 layout
    std::int32_t  padding3;        ///< Padding to match GLSL std430 layout
};

/**
 * @brief CPU-side mirror of the complete GPU material struct.
 *
 * This must match mat.glsl:
 *
 * struct GPU_Material {
 *     GPU_ColorComponent base_color;
 *     GPU_ColorComponent emission;
 *     GPU_ColorComponent sheen_color;
 *
 *     GPU_ScalarComponent roughness;
 *     GPU_ScalarComponent metallic;
 *     GPU_ScalarComponent specular;
 *     GPU_ScalarComponent specular_tint;
 *     GPU_ScalarComponent transmission;
 *     GPU_ScalarComponent transmission_roughness;
 *     GPU_ScalarComponent clearcoat;
 *     GPU_ScalarComponent clearcoat_roughness;
 *     GPU_ScalarComponent subsurface;
 *     GPU_ScalarComponent sheen;
 *     GPU_ScalarComponent sheen_tint;
 *     GPU_ScalarComponent anisotropy;
 *
 *     float ior;
 *     float anisotropy_rotation;
 *     float padding1;
 *     float padding2;
 *
 *     GPU_TextureComponent normal_map;
 *     GPU_TextureComponent tangent_map;
 * };
 *
 * Any change to the GLSL struct must be mirrored here and vice versa.
 */
struct GPU_Material {
    GPU_ColorComponent base_color;
    GPU_ColorComponent emission;
    GPU_ColorComponent sheen_color;

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
    GPU_TextureComponent tangent_map;
};
