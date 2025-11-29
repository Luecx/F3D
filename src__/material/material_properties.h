#pragma once

#include "material_components.h"

struct MaterialProperties {
    ColorComponent base_color;
    ColorComponent subsurface_color;
    ColorComponent subsurface_radius;
    ColorComponent emission_color;

    FloatComponent metallic;
    FloatComponent specular;
    FloatComponent specular_tint;
    FloatComponent roughness;
    FloatComponent anisotropic;
    FloatComponent anisotropic_rotation;
    FloatComponent subsurface;
    FloatComponent sheen;
    FloatComponent sheen_tint;
    FloatComponent clearcoat;
    FloatComponent clearcoat_roughness;
    FloatComponent ior;
    FloatComponent transmission;
    FloatComponent transmission_roughness;
    FloatComponent emission_strength;

    std::shared_ptr<TextureResource> normal_map;
    std::shared_ptr<TextureResource> displacement_map;
    std::shared_ptr<TextureResource> ambient_occlusion_map;

    void set_defaults();

    void print_overview() const;

    bool is_transparent(float threshold = 0.001f) const;
};
