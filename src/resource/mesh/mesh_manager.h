#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "mesh.h"
#include "../resource_state.h"

/**
 * @brief Manages deduplicated mesh resources by path.
 *
 * Provides path-based lookup, reference-counted request/release,
 * and state dumping for debugging.
 */
class MeshManager {
  public:
    MeshManager() = default;
    ~MeshManager() = default;

    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager&) = delete;

    /**
     * @brief Get or create a mesh for a given path.
     */
    std::shared_ptr<Mesh> get(const std::string& path);

    /**
     * @brief Request a state for the mesh at @p path.
     */
    void request(const std::string& path, resources::ResourceState state);

    /**
     * @brief Release a state for the mesh at @p path.
     */
    void release(const std::string& path, resources::ResourceState state);

    /**
     * @brief Dump the state of all meshes with indentation.
     */
    void dump_state(int indent = 0) const;

  private:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes_;
    mutable std::mutex mtx_;
};
