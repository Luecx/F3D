#include "material_data.h"

#include "resource_manager.h"

#include "../logging/logging.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace logging;

namespace {
std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool parse_vec3(std::istringstream& iss, float& x, float& y, float& z) { return static_cast<bool>(iss >> x >> y >> z); }

float convert_ns_to_roughness(float ns) {
    float normalized = std::clamp(ns / 1000.0f, 0.0f, 1.0f);
    return std::sqrt(std::max(1.0f - normalized, 0.0f));
}

} // namespace

MaterialData::MaterialData(const std::string& path, std::string material_name)
    : ResourceData(path), material_name_(std::move(material_name)) {
    set_label("Material");
}

bool MaterialData::load_to_ram() {
    std::ifstream file(get_path());
    if (!file.is_open()) {
        log(1, ERROR, "Failed to open material file: " + get_path());
        return false;
    }

    material = std::make_shared<Material>(material_name_);
    material->set_default_material();
    auto& props = material->properties();

    auto base_dir = std::filesystem::path(get_path()).parent_path();

    auto resolve_texture = [&](const std::string& relative) -> std::shared_ptr<TextureResource> {
        if (relative.empty()) {
            return nullptr;
        }
        std::filesystem::path full = base_dir / relative;
        if (auto* mgr = get_manager()) {
            return mgr->get_texture(full.string());
        }
        return nullptr;
    };

    bool found = false;
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        if (keyword == "newmtl") {
            std::string name;
            std::getline(iss, name);
            name = trim(name);
            if (found) {
                break; // finished parsing target material
            }
            if (name == material_name_) {
                found = true;
            }
            continue;
        }

        if (!found) {
            continue;
        }

        if (keyword == "Kd") {
            float r, g, b;
            if (parse_vec3(iss, r, g, b)) {
                props.base_color.set_constant(r, g, b);
            }
        } else if (keyword == "Ps" || keyword == "Ks") {
            float r, g, b;
            if (parse_vec3(iss, r, g, b)) {
                props.specular.set_constant((r + g + b) / 3.0f);
            }
        } else if (keyword == "Ke") {
            float r, g, b;
            if (parse_vec3(iss, r, g, b)) {
                props.emission_color.set_constant(r, g, b);
            }
        } else if (keyword == "Ns") {
            float ns;
            if (iss >> ns) {
                props.roughness.set_constant(convert_ns_to_roughness(ns));
            }
        } else if (keyword == "d") {
            float dissolve;
            if (iss >> dissolve) {
                props.transmission.set_constant(1.0f - dissolve);
            }
        } else if (keyword == "Tr") {
            float tr;
            if (iss >> tr) {
                props.transmission.set_constant(tr);
            }
        } else if (keyword == "map_Kd") {
            std::string tex;
            if (iss >> tex) {
                props.base_color.set_texture(resolve_texture(tex));
            }
        } else if (keyword == "map_Ks") {
            std::string tex;
            if (iss >> tex) {
                props.specular.set_texture(resolve_texture(tex));
            }
        } else if (keyword == "map_Ke") {
            std::string tex;
            if (iss >> tex) {
                props.emission_color.set_texture(resolve_texture(tex));
            }
        } else if (keyword == "map_Bump" || keyword == "bump") {
            std::string tex;
            if (iss >> tex) {
                material->assign_texture_slot("normal_map", resolve_texture(tex));
            }
        } else if (keyword == "map_d") {
            std::string tex;
            if (iss >> tex) {
                props.transmission.set_texture(resolve_texture(tex));
            }
        }
    }

    if (!found) {
        log(1, WARNING, "Material '" + material_name_ + "' not found in " + get_path());
        material.reset();
        return false;
    }

    return true;
}

void MaterialData::unload_from_ram() {
    material.reset();
    gpu_material_index_ = -1;
}

bool MaterialData::load_to_gpu() {
    if (!material) {
        return false;
    }

    auto require_texture = [](const std::shared_ptr<TextureResource>& tex) {
        if (tex) {
            tex->require(resources::ResourceState::Gpu);
        }
    };

    auto& props = material->properties();

    require_texture(props.normal_map);
    require_texture(props.displacement_map);
    require_texture(props.ambient_occlusion_map);

    auto handle_component = [&](auto& component) {
        if (component.mode == ComponentMode::TEXTURE && component.texture) {
            component.texture->require(resources::ResourceState::Gpu);
        }
    };

    handle_component(props.base_color);
    handle_component(props.subsurface_color);
    handle_component(props.subsurface_radius);
    handle_component(props.emission_color);

    handle_component(props.metallic);
    handle_component(props.specular);
    handle_component(props.specular_tint);
    handle_component(props.roughness);
    handle_component(props.anisotropic);
    handle_component(props.anisotropic_rotation);
    handle_component(props.subsurface);
    handle_component(props.sheen);
    handle_component(props.sheen_tint);
    handle_component(props.clearcoat);
    handle_component(props.clearcoat_roughness);
    handle_component(props.ior);
    handle_component(props.transmission);
    handle_component(props.transmission_roughness);
    handle_component(props.emission_strength);

    int new_index = -1;
    if (auto* mgr = get_manager()) {
        if (auto* mat_manager = mgr->material_manager()) {
            new_index = static_cast<int>(mat_manager->add_material(material));
        }
    }
    gpu_material_index_ = new_index;

    return true;
}

void MaterialData::unload_from_gpu() {
    if (!material) {
        return;
    }

    auto release_texture = [](const std::shared_ptr<TextureResource>& tex) {
        if (tex) {
            tex->release(resources::ResourceState::Gpu);
        }
    };

    auto& props = material->properties();

    release_texture(props.normal_map);
    release_texture(props.displacement_map);
    release_texture(props.ambient_occlusion_map);

    auto release_component = [&](auto& component) {
        if (component.mode == ComponentMode::TEXTURE && component.texture) {
            component.texture->release(resources::ResourceState::Gpu);
        }
    };

    release_component(props.base_color);
    release_component(props.subsurface_color);
    release_component(props.subsurface_radius);
    release_component(props.emission_color);

    release_component(props.metallic);
    release_component(props.specular);
    release_component(props.specular_tint);
    release_component(props.roughness);
    release_component(props.anisotropic);
    release_component(props.anisotropic_rotation);
    release_component(props.subsurface);
    release_component(props.sheen);
    release_component(props.sheen_tint);
    release_component(props.clearcoat);
    release_component(props.clearcoat_roughness);
    release_component(props.ior);
    release_component(props.transmission);
    release_component(props.transmission_roughness);
    release_component(props.emission_strength);
    gpu_material_index_ = -1;
}

bool MaterialData::is_transparent(float threshold) const {
    if (!material) {
        return false;
    }
    return material->properties().is_transparent(threshold);
}
