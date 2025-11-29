#pragma once

#include "image_data.h"
#include "loading_thread.h"
#include "material_data.h"
#include "mesh_data.h"
#include "texture_resource.h"
#include "../material/material_manager.h"

#include <filesystem>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>

class ResourceManager {
  public:
    ResourceManager();
    ~ResourceManager();

    std::shared_ptr<ImageData> get_image(const std::string& path);
    std::shared_ptr<TextureResource> get_texture(const std::string& path);
    std::shared_ptr<MaterialData> get_material(const std::string& mtl_path, const std::string& material_name);
    std::shared_ptr<MeshData> get_mesh(const std::string& path);

    MaterialManager* material_manager();
    const MaterialManager* material_manager() const;

#ifdef F3D_PARALLEL_LOADING
    LoadingThread loading_thread;
#endif

  private:
    using Path = std::filesystem::path;

    Path canonical_path(const std::string& path) const;
    std::string material_key(const Path& mtl_path, const std::string& material_name) const;

    std::unordered_map<Path, std::shared_ptr<ImageData>> images_;
    std::unordered_map<Path, std::shared_ptr<TextureResource>> textures_;
    std::unordered_map<std::string, std::shared_ptr<MaterialData>> materials_;
    std::unordered_map<Path, std::shared_ptr<MeshData>> meshes_;

    std::unique_ptr<MaterialManager> material_manager_;

    friend std::ostream& operator<<(std::ostream& os, const ResourceManager& manager);
};
