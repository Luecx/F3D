#include "material_properties.h"

#include <iostream>

void MaterialProperties::set_defaults() {
    // --- Colors ---
    base_color.set_constant(1.0f, 1.0f, 1.0f);
    emission_color.set_constant(0.0f, 0.0f, 0.0f);
    sheen_color.set_constant(1.0f, 1.0f, 1.0f);

    // --- Scalars (values similar to common Disney/Principled defaults) ---
    roughness.set_constant(0.5f);
    metallic.set_constant(0.0f);
    specular.set_constant(0.5f);
    specular_tint.set_constant(0.0f);
    transmission.set_constant(0.0f);
    transmission_roughness.set_constant(0.0f);
    clearcoat.set_constant(0.0f);
    clearcoat_roughness.set_constant(0.03f);
    subsurface.set_constant(0.0f);
    sheen.set_constant(0.0f);
    sheen_tint.set_constant(0.5f);
    anisotropic.set_constant(0.0f);

    ior.set_constant(1.5f);
    anisotropic_rotation.set_constant(0.0f);

    // --- Texture-only slots ---
    normal_map.reset();
    tangent_map.reset();
}

void MaterialProperties::print_overview() const {
    std::cout << "MaterialProperties overview:\n";

    base_color.print("base_color");
    emission_color.print("emission_color");
    sheen_color.print("sheen_color");

    roughness.print("roughness");
    metallic.print("metallic");
    specular.print("specular");
    specular_tint.print("specular_tint");
    transmission.print("transmission");
    transmission_roughness.print("transmission_roughness");
    clearcoat.print("clearcoat");
    clearcoat_roughness.print("clearcoat_roughness");
    subsurface.print("subsurface");
    sheen.print("sheen");
    sheen_tint.print("sheen_tint");
    anisotropic.print("anisotropic");

    ior.print("ior");
    anisotropic_rotation.print("anisotropic_rotation");

    print_texture_slot("normal_map", normal_map);
    print_texture_slot("tangent_map", tangent_map);
}

bool MaterialProperties::is_transparent(float threshold) const {
    auto component_is_transparent = [&](const FloatComponent& comp) {
        if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
            // Presence of a transmission texture suggests possible transparency.
            return true;
        }
        return (comp.mode == ComponentMode::CONSTANT && comp.value > threshold);
    };

    if (component_is_transparent(transmission)) {
        return true;
    }
    return false;
}
