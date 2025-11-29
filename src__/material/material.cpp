#include "material.h"

Material::Material() { properties_.set_defaults(); }

Material::Material(std::string name) : name_(std::move(name)) { properties_.set_defaults(); }

void Material::set_default_material() { properties_.set_defaults(); }

MaterialProperties& Material::properties() { return properties_; }

const MaterialProperties& Material::properties() const { return properties_; }

void Material::set_float_property(const std::string& name, float value,
                                  const std::shared_ptr<TextureResource>& texture) {
    if (auto* component = float_component(name)) {
        if (texture) {
            component->set_texture(texture);
        } else {
            component->set_constant(value);
        }
    }
}

void Material::set_color_property(const std::string& name, float r, float g, float b,
                                  const std::shared_ptr<TextureResource>& texture) {
    if (auto* component = color_component(name)) {
        if (texture) {
            component->set_texture(texture);
        } else {
            component->set_constant(r, g, b);
        }
    }
}

void Material::assign_texture_slot(const std::string& name, const std::shared_ptr<TextureResource>& texture) {
    if (name == "normal_map") {
        properties_.normal_map = texture;
    } else if (name == "displacement_map") {
        properties_.displacement_map = texture;
    } else if (name == "ambient_occlusion_map") {
        properties_.ambient_occlusion_map = texture;
    }
}

void Material::print_overview() const { properties_.print_overview(); }

FloatComponent* Material::float_component(const std::string& name) {
    if (name == "metallic")
        return &properties_.metallic;
    if (name == "specular")
        return &properties_.specular;
    if (name == "specular_tint")
        return &properties_.specular_tint;
    if (name == "roughness")
        return &properties_.roughness;
    if (name == "anisotropic")
        return &properties_.anisotropic;
    if (name == "anisotropic_rotation")
        return &properties_.anisotropic_rotation;
    if (name == "subsurface")
        return &properties_.subsurface;
    if (name == "sheen")
        return &properties_.sheen;
    if (name == "sheen_tint")
        return &properties_.sheen_tint;
    if (name == "clearcoat")
        return &properties_.clearcoat;
    if (name == "clearcoat_roughness")
        return &properties_.clearcoat_roughness;
    if (name == "ior")
        return &properties_.ior;
    if (name == "transmission")
        return &properties_.transmission;
    if (name == "transmission_roughness")
        return &properties_.transmission_roughness;
    if (name == "emission_strength")
        return &properties_.emission_strength;
    return nullptr;
}

ColorComponent* Material::color_component(const std::string& name) {
    if (name == "base_color")
        return &properties_.base_color;
    if (name == "subsurface_color")
        return &properties_.subsurface_color;
    if (name == "subsurface_radius")
        return &properties_.subsurface_radius;
    if (name == "emission_color")
        return &properties_.emission_color;
    return nullptr;
}
