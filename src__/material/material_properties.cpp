#include "material_properties.h"

#include <iostream>

void MaterialProperties::set_defaults() {
    base_color.set_constant(1.0f, 1.0f, 1.0f);
    subsurface_color.set_constant(1.0f, 1.0f, 1.0f);
    subsurface_radius.set_constant(1.0f, 1.0f, 1.0f);
    emission_color.set_constant(0.0f, 0.0f, 0.0f);

    metallic.set_constant(0.0f);
    specular.set_constant(0.5f);
    specular_tint.set_constant(0.0f);
    roughness.set_constant(0.5f);
    anisotropic.set_constant(0.0f);
    anisotropic_rotation.set_constant(0.0f);
    subsurface.set_constant(0.0f);
    sheen.set_constant(0.0f);
    sheen_tint.set_constant(0.5f);
    clearcoat.set_constant(0.0f);
    clearcoat_roughness.set_constant(0.03f);
    ior.set_constant(1.45f);
    transmission.set_constant(0.0f);
    transmission_roughness.set_constant(0.0f);
    emission_strength.set_constant(0.0f);

    normal_map.reset();
    displacement_map.reset();
    ambient_occlusion_map.reset();
}

void MaterialProperties::print_overview() const {
    std::cout << "\n=== Material Overview ===\n";
    base_color.print("base_color");
    subsurface_color.print("subsurface_color");
    subsurface_radius.print("subsurface_radius");
    emission_color.print("emission_color");

    metallic.print("metallic");
    specular.print("specular");
    specular_tint.print("specular_tint");
    roughness.print("roughness");
    anisotropic.print("anisotropic");
    anisotropic_rotation.print("anisotropic_rotation");
    subsurface.print("subsurface");
    sheen.print("sheen");
    sheen_tint.print("sheen_tint");
    clearcoat.print("clearcoat");
    clearcoat_roughness.print("clearcoat_roughness");
    ior.print("ior");
    transmission.print("transmission");
    transmission_roughness.print("transmission_roughness");
    emission_strength.print("emission_strength");

    print_texture_slot("normal_map", normal_map);
    print_texture_slot("displacement_map", displacement_map);
    print_texture_slot("ambient_occlusion_map", ambient_occlusion_map);
    std::cout << "==========================\n";
}

bool MaterialProperties::is_transparent(float threshold) const {
    auto component_is_transparent = [&](const FloatComponent& comp) {
        if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
            return true;
        }
        return comp.mode == ComponentMode::CONSTANT && comp.value > threshold;
    };

    if (component_is_transparent(transmission)) {
        return true;
    }
    return false;
}
