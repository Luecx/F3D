#include "material_properties.h"

void MaterialProperties::set_defaults() {
    base_color.set_constant(1.0f, 1.0f, 1.0f);
    subsurface_color.set_constant(1.0f, 1.0f, 1.0f);
    subsurface_radius.set_constant(1.0f, 1.0f, 1.0f);
    emission_color.set_constant(0.0f, 0.0f, 0.0f);
    sheen_color.set_constant(0.0f, 0.0f, 0.0f);

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
