#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "mesh.h"
#include "../resource_state.h"

class MeshManager {
    public:
    MeshManager(TextureManager* tex_mgr = nullptr,
                MaterialManager* mat_mgr = nullptr);

    std::shared_ptr<Mesh> get(const std::string& path);

    void require(const std::string& path, ResourceState state);
    void release(const std::string& path, ResourceState state);

    void dump_state(int indent = 0) const;

    private:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes_;
    mutable std::mutex mtx_;
    TextureManager*  tex_mgr_;
    MaterialManager* mat_mgr_;
};
