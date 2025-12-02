#pragma once

#include <memory>

#include "material_components.h"

struct MaterialProperties {
    // Color-like components
    ColorComponent base_color;
    ColorComponent emission_color;
    ColorComponent sheen_color;
    ColorComponent subsurface_color;
    ColorComponent subsurface_radius;

    // Scalar components
    FloatComponent roughness;
    FloatComponent metallic;
    FloatComponent specular;
    FloatComponent specular_tint;
    FloatComponent transmission;
    FloatComponent transmission_roughness;
    FloatComponent clearcoat;
    FloatComponent clearcoat_roughness;
    FloatComponent subsurface;
    FloatComponent sheen;
    FloatComponent sheen_tint;
    FloatComponent anisotropic;
    FloatComponent ior;
    FloatComponent anisotropic_rotation;
    FloatComponent emission_strength;

    // Standalone texture slots
    std::shared_ptr<Texture> normal_map;
    std::shared_ptr<Texture> displacement_map;
    std::shared_ptr<Texture> ambient_occlusion_map;

    void set_defaults();
};
