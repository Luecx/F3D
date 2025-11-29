#pragma once

#include <memory>
#include <vector>

#include "mesh.h"
#include "mesh_group_gpu_data.h"
#include "../resource_types.h"

/**
 * @brief A group (chunk) of meshes with a single aggregated GPU buffer.
 *
 * A MeshGroup is your streaming / chunk abstraction. It:
 *  - Holds a set of CPU-side Mesh objects (Drive/Ram states).
 *  - Owns exactly one MeshGroupGPUData instance (Gpu state) that packs
 *    all meshes into a single VAO/VBO/EBO for efficient rendering.
 *
 * Typical usage:
 *  - Add meshes during level loading.
 *  - Call request_state(Ram) to ensure CPU data is available.
 *  - Call request_state(Gpu) to build the aggregated GPU buffers.
 *  - Call release_state(Gpu) / release_state(Ram) for streaming.
 */
class MeshGroup {
  public:
    using SPtr = std::shared_ptr<MeshGroup>;

    MeshGroup() = default;

    /**
     * @brief Adds a mesh to the group.
     *
     * The mesh's CPU data will be included in rebuild_from_meshes() once
     * the group is promoted to GPU state.
     */
    void add_mesh(const Mesh::SPtr& mesh);

    /**
     * @brief Removes a mesh from the group (if present).
     *
     * Does not modify GPU buffers immediately; you typically call
     * request_state(Gpu) again (which rebuilds the group) after changing
     * the set of meshes.
     */
    void remove_mesh(const Mesh::SPtr& mesh);

    /// Returns the list of meshes in this group.
    const std::vector<Mesh::SPtr>& meshes() const { return meshes_; }

    /// Returns the aggregated GPU data for this group (may be nullptr).
    std::shared_ptr<MeshGroupGPUData> gpu_data() const { return gpu_data_; }

    // ------------------------------------------------------------------
    // Resource state API (Drive/Ram/Gpu on group level).
    // ------------------------------------------------------------------

    /**
     * @brief Request that the group enters a given state.
     *
     * Semantics:
     *  - Drive: no-op.
     *  - Ram:   calls Mesh::request_state(Ram) for all meshes.
     *  - Gpu:   ensures Ram for all meshes, then rebuilds aggregated
     *           GPU buffers via MeshGroupGPUData::rebuild_from_meshes().
     */
    void request_state(resources::ResourceState state);

    /**
     * @brief Release a previously requested state.
     *
     * Semantics:
     *  - Drive: no-op.
     *  - Gpu:   releases the aggregated GPU buffers.
     *  - Ram:   calls Mesh::release_state(Ram) for all meshes.
     */
    void release_state(resources::ResourceState state);

    /**
     * @brief Returns whether the group is considered to be in a state.
     *
     * Semantics:
     *  - Drive: always true.
     *  - Ram:   true if request_state(Ram) was called and not released.
     *  - Gpu:   true if aggregated GPU buffers currently exist.
     */
    bool is_in_state(resources::ResourceState state) const;

  private:
    std::vector<Mesh::SPtr> meshes_;
    std::shared_ptr<MeshGroupGPUData> gpu_data_;

    bool has_ram_{false};
    bool has_gpu_{false};
};
