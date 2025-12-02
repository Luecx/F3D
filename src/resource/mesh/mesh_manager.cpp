#include "mesh_manager.h"

#include "../textures/texture_manager.h"
#include "../materials/material_manager.h"
#include "../../logging/logging.h"
#include "../resource_logging.h"

MeshManager::MeshManager(TextureManager* tex_mgr,
                         MaterialManager* mat_mgr)
    : tex_mgr_(tex_mgr)
    , mat_mgr_(mat_mgr) {}

std::shared_ptr<Mesh> MeshManager::get(const std::string& path) {
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = meshes_.find(path);
    if (it != meshes_.end()) {
        return it->second;
    }

    auto mesh = std::make_shared<Mesh>(path, tex_mgr_, mat_mgr_);
    meshes_[path] = mesh;
    return mesh;
}

void MeshManager::require(const std::string& path, ResourceState state) {
    auto mesh = get(path);
    if (!mesh) return;

    // Mesh is CPU-only by design, but we allow require(Drive) or require(Cpu).
    if (state == ResourceState::Gpu) {
        logging::log(reslog::MESH, logging::ERROR,
                     "MeshManager::require(Gpu) called for '" + path +
                         "'. Mesh is CPU-only; use MeshGroup for GPU geometry.");
        return;
    }

    mesh->require(state);
}

void MeshManager::release(const std::string& path, ResourceState state) {
    std::shared_ptr<Mesh> mesh;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = meshes_.find(path);
        if (it != meshes_.end()) {
            mesh = it->second;
        }
    }

    if (!mesh) return;

    if (state == ResourceState::Gpu) {
        logging::log(reslog::MESH, logging::ERROR,
                     "MeshManager::release(Gpu) called for '" + path +
                         "'. Mesh is CPU-only; use MeshGroup for GPU geometry.");
        return;
    }

    mesh->release(state);
}

void MeshManager::dump_state(int indent) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string pad(indent, ' ');

    logging::log(reslog::MESH, logging::INFO, pad + "Meshes:");
    for (const auto& [path, mesh] : meshes_) {
        logging::log(
            reslog::MESH,
            logging::INFO,
            pad + "  " + path +
                " cpu_users=" + std::to_string(mesh->cpu_users()) +
                " gpu_users=" + std::to_string(mesh->gpu_users())
        );
    }
}
