#include "mesh_manager.h"

#include "../resource_logging.h"
#include "../../logging/logging.h"

std::shared_ptr<Mesh> MeshManager::get(const std::string& path) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = meshes_.find(path);
    if (it != meshes_.end()) {
        logging::log(reslog::MESH, logging::DEBUG, "MeshManager hit: " + path);
        return it->second;
    }
    auto mesh = std::make_shared<Mesh>(path);
    meshes_[path] = mesh;
    logging::log(reslog::MESH, logging::INFO, "MeshManager created mesh for path: " + path);
    return mesh;
}

void MeshManager::request(const std::string& path, resources::ResourceState state) {
    auto mesh = get(path);
    if (mesh) {
        mesh->request(state);
    }
}

void MeshManager::release(const std::string& path, resources::ResourceState state) {
    std::shared_ptr<Mesh> mesh;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = meshes_.find(path);
        if (it == meshes_.end()) {
            logging::log(reslog::MESH, logging::WARNING, "MeshManager release on unknown path: " + path);
            return;
        }
        mesh = it->second;
    }
    if (mesh) {
        mesh->release(state);
    }
}

void MeshManager::dump_state(int indent) const {
    std::lock_guard<std::mutex> lock(mtx_);
    const std::string pad(indent, ' ');
    logging::log(reslog::MESH, logging::INFO, pad + "MeshManager state:");
    for (const auto& [path, mesh] : meshes_) {
        if (!mesh) continue;
        logging::log(reslog::MESH, logging::INFO,
                     pad + "  " + path +
                         " RAM=" + (mesh->has_ram() ? "yes" : "no") +
                         " rc_ram=" + std::to_string(mesh->ram_refcount()));
    }
}
