#include "resource_manager.h"

#include "../logging/logging.h"

#include <filesystem>
#include <iomanip>
#include <ostream>
#include <type_traits>

#include <glad/glad.h>

using namespace logging;

namespace {
std::string canonical_string(const std::filesystem::path& path) {
    return std::filesystem::weakly_canonical(path).string();
}
} // namespace

ResourceManager::ResourceManager() { log(1, INFO, "ResourceManager created"); }

ResourceManager::~ResourceManager() { log(1, INFO, "ResourceManager destroyed"); }

ResourceManager::Path ResourceManager::canonical_path(const std::string& path) const {
    std::filesystem::path p(path);
    if (p.is_relative()) {
        p = std::filesystem::current_path() / p;
    }
    return std::filesystem::weakly_canonical(p);
}

std::string ResourceManager::material_key(const Path& mtl_path, const std::string& material_name) const {
    return canonical_string(mtl_path) + "#" + material_name;
}

std::shared_ptr<ImageData> ResourceManager::get_image(const std::string& path) {
    auto abs = canonical_path(path);
    if (auto it = images_.find(abs); it != images_.end()) {
        return it->second;
    }
    auto image = std::make_shared<ImageData>(abs.string());
    image->set_manager(this);
    images_.emplace(abs, image);
    return image;
}

std::shared_ptr<TextureResource> ResourceManager::get_texture(const std::string& path) {
    auto abs = canonical_path(path);
    if (auto it = textures_.find(abs); it != textures_.end()) {
        return it->second;
    }

    TextureSpecification spec{};
    spec.type = TextureType::TEX_2D;
    spec.internal_format = GL_RGBA8;
    spec.data_format = GL_RGBA;
    spec.data_type = GL_UNSIGNED_BYTE;
    spec.wrap_s = GL_REPEAT;
    spec.wrap_t = GL_REPEAT;
    spec.wrap_r = GL_REPEAT;
    spec.min_filter = GL_LINEAR_MIPMAP_LINEAR;
    spec.mag_filter = GL_LINEAR;
    spec.generate_mipmaps = true;

    auto image = get_image(abs.string());
    auto texture = std::make_shared<TextureResource>(abs.string(), image, spec);
    texture->set_manager(this);
    textures_.emplace(abs, texture);
    return texture;
}

std::shared_ptr<MaterialData> ResourceManager::get_material(const std::string& mtl_path,
                                                            const std::string& material_name) {
    auto abs = canonical_path(mtl_path);
    auto key = material_key(abs, material_name);
    if (auto it = materials_.find(key); it != materials_.end()) {
        return it->second;
    }
    auto material = std::make_shared<MaterialData>(abs.string(), material_name);
    material->set_manager(this);
    materials_.emplace(key, material);
    return material;
}

std::shared_ptr<MeshData> ResourceManager::get_mesh(const std::string& path) {
    auto abs = canonical_path(path);
    if (auto it = meshes_.find(abs); it != meshes_.end()) {
        return it->second;
    }
    auto mesh = std::make_shared<MeshData>(abs.string());
    mesh->set_manager(this);
    meshes_.emplace(abs, mesh);
    return mesh;
}

MaterialManager* ResourceManager::material_manager() {
    if (!material_manager_) {
        material_manager_ = std::make_unique<MaterialManager>();
    }
    return material_manager_.get();
}

const MaterialManager* ResourceManager::material_manager() const { return material_manager_.get(); }

namespace {
template<typename Key> std::string to_string_key(const Key& key) {
    if constexpr (std::is_same_v<Key, std::filesystem::path>) {
        return key.string();
    } else {
        return key;
    }
}

void print_requests(std::ostream& os, const std::array<std::size_t, resources::kResourceStateCount>& counts) {
    os << "requests=";
    os << "{";
    for (std::size_t i = 0; i < counts.size(); ++i) {
        auto state = static_cast<resources::ResourceState>(i);
        os << resources::to_string(state) << ": " << counts[i];
        if (i + 1 < counts.size()) {
            os << ", ";
        }
    }
    os << "}";
}

template<typename Map> void print_section(std::ostream& os, const Map& map, const std::string& label) {
    os << label << " (" << map.size() << ")\n";
    if (map.empty()) {
        os << "  <none>\n";
        return;
    }
    std::size_t index = 0;
    for (const auto& entry : map) {
        const auto& key = entry.first;
        const auto& handle = entry.second;
        os << "  [" << index++ << "] " << to_string_key(key) << "\n";
        if (!handle) {
            os << "     <null handle>\n";
            continue;
        }
        os << "     state=" << resources::to_string(handle->current_state()) << ", use_count=" << handle.use_count()
           << ", ";
        print_requests(os, handle->request_counts());
        os << "\n";

        auto deps = handle->active_dependencies();
        if (!deps.empty()) {
            os << "     dependencies (" << deps.size() << ")\n";
            for (const auto& dep : deps) {
                os << "        owning=" << resources::to_string(dep.owning_state) << " -> " << dep.path << " @"
                   << resources::to_string(dep.required_state) << "\n";
            }
        }
    }
}
} // namespace

std::ostream& operator<<(std::ostream& os, const ResourceManager& manager) {
    os << "ResourceManager State\n";
    print_section(os, manager.images_, "Images");
    print_section(os, manager.textures_, "Textures");
    print_section(os, manager.materials_, "Materials");
    print_section(os, manager.meshes_, "Meshes");
    return os;
}
