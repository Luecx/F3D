#pragma once

#include "material_components.h"

/**
 * @brief High-level material parameter set for a principled BSDF.
 *
 * This is the CPU-side representation of a material. It combines several
 * color and scalar components, plus a few dedicated texture slots.
 *
 * The layout here does NOT need to match the GPU layout 1:1; instead,
 * MaterialManager converts this structure into GPU_Material (see
 * material_gpu.h), which mirrors the GLSL struct in mat.glsl.
 */
struct MaterialProperties {
    // --- Color-like components (mapped to GPU_ColorComponent) ---
    ColorComponent base_color;     ///< Base/albedo color.
    ColorComponent emission_color; ///< Emissive color.
    ColorComponent sheen_color;    ///< Sheen lobe color.

    // --- Scalar components (mapped to GPU_ScalarComponent) ---
    FloatComponent roughness;              ///< Microfacet roughness.
    FloatComponent metallic;               ///< Metallic factor [0,1].
    FloatComponent specular;               ///< Specular level.
    FloatComponent specular_tint;          ///< Tint of the specular lobe.
    FloatComponent transmission;           ///< Transmission / opacity.
    FloatComponent transmission_roughness; ///< Roughness for transmitted light.
    FloatComponent clearcoat;              ///< Clearcoat intensity.
    FloatComponent clearcoat_roughness;    ///< Clearcoat roughness.
    FloatComponent subsurface;             ///< Subsurface scattering weight.
    FloatComponent sheen;                  ///< Sheen strength.
    FloatComponent sheen_tint;             ///< Sheen tint.
    FloatComponent anisotropic;            ///< Anisotropy strength.

    // --- Additional scalar properties used directly on GPU ---
    FloatComponent ior;                    ///< Index of refraction.
    FloatComponent anisotropic_rotation;   ///< Rotation of anisotropy.

    // --- Texture-only slots (mapped to GPU_TextureComponent) ---
    std::shared_ptr<Texture> normal_map;   ///< Normal map in tangent space.
    std::shared_ptr<Texture> tangent_map;  ///< Optional tangent map (or extra slot).

    /**
     * @brief Initializes all components to "reasonable" default values
     *        for a neutral, mostly-diffuse material.
     */
    void set_defaults();

    /**
     * @brief Prints a human-readable overview of all components.
     *
     * This is useful mainly for debugging and tooling.
     */
    void print_overview() const;

    /**
     * @brief Returns true if this material should be considered transparent
     *        for the purpose of sorting / render path selection.
     *
     * The decision is currently based on the transmission component:
     *  - If it is in TEXTURE mode and has a texture, we treat the material
     *    as potentially transparent.
     *  - If it is a constant and exceeds @p threshold, it is also treated
     *    as transparent.
     *
     * @param threshold Constant transmission threshold above which the
     *                  material is considered transparent.
     */
    [[nodiscard]] bool is_transparent(float threshold = 0.001f) const;
};
