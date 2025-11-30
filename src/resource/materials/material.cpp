#include "material.h"

#include <vector>

#include "../textures/texture.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

using resources::ResourceState;

namespace {

// Collect all textures referenced by a ColorComponent.
void collect_textures(const ColorComponent& comp,
                      std::vector<std::shared_ptr<Texture>>& out) {
    if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
        out.push_back(comp.texture);
    }
}

// Collect all textures referenced by a FloatComponent.
void collect_textures(const FloatComponent& comp,
                      std::vector<std::shared_ptr<Texture>>& out) {
    if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
        out.push_back(comp.texture);
    }
}

// Collect all textures from the full MaterialProperties.
void collect_textures(const MaterialProperties& props,
                      std::vector<std::shared_ptr<Texture>>& out) {
    // Color components
    collect_textures(props.base_color, out);
    collect_textures(props.emission_color, out);
    collect_textures(props.sheen_color, out);

    // Scalar components
    collect_textures(props.roughness, out);
    collect_textures(props.metallic, out);
    collect_textures(props.specular, out);
    collect_textures(props.specular_tint, out);
    collect_textures(props.transmission, out);
    collect_textures(props.transmission_roughness, out);
    collect_textures(props.clearcoat, out);
    collect_textures(props.clearcoat_roughness, out);
    collect_textures(props.subsurface, out);
    collect_textures(props.sheen, out);
    collect_textures(props.sheen_tint, out);
    collect_textures(props.anisotropic, out);

    // Scalars used directly (ior, anisotropic_rotation) may also be texture-driven.
    collect_textures(props.ior, out);
    collect_textures(props.anisotropic_rotation, out);

    // Texture-only slots
    if (props.normal_map) {
        out.push_back(props.normal_map);
    }
    if (props.tangent_map) {
        out.push_back(props.tangent_map);
    }
}

} // namespace

Material::Material(std::string name)
    : name_(std::move(name)) {
    properties_.set_defaults();
}

void Material::request(ResourceState state) {
    logging::log(reslog::MATERIAL, logging::INFO,
                 "Requesting material '" + name_ + "' to be in " + std::string(resources::to_string(state)));
    if (state == ResourceState::Drive) {
        // Nothing to do; Drive is our implicit baseline.
        return;
    }

    std::vector<std::shared_ptr<Texture>> textures;
    collect_textures(properties_, textures);

    for (auto& tex : textures) {
        if (tex) {
            tex->request(state);
        }
    }
}

void Material::release(ResourceState state) {
    logging::log(reslog::MATERIAL, logging::INFO,
                 "Releasing material '" + name_ + "' from " + std::string(resources::to_string(state)));
    if (state == ResourceState::Drive) {
        // Nothing to do for Drive.
        return;
    }

    std::vector<std::shared_ptr<Texture>> textures;
    collect_textures(properties_, textures);

    for (auto& tex : textures) {
        if (tex) {
            tex->release(state);
        }
    }
}

bool Material::is_in_state(ResourceState state) const {
    if (state == ResourceState::Drive) {
        // Materials always "exist" on Drive by definition.
        return true;
    }

    std::vector<std::shared_ptr<Texture>> textures;
    collect_textures(properties_, textures);

    if (textures.empty()) {
        // No textures -> material is effectively always available.
        return true;
    }

    if (state == ResourceState::Ram) {
        // RAM: allow textures that are already in GPU as well.
        for (const auto& tex : textures) {
            if (!tex) {
                continue;
            }
            if (!tex->is_in_state(ResourceState::Ram) &&
                !tex->is_in_state(ResourceState::Gpu)) {
                return false;
            }
        }
        return true;
    }

    if (state == ResourceState::Gpu) {
        for (const auto& tex : textures) {
            if (!tex) {
                continue;
            }
            if (!tex->is_in_state(ResourceState::Gpu)) {
                return false;
            }
        }
        return true;
    }

    return false;
}
