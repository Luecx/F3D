#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "submesh.h"
#include "../../gldata/vao_data.h"
#include "../../gldata/vbo_data.h"

class Mesh;
struct MeshCPUData;
class Material;

/**
 * @brief Draw description for a single submesh within a MeshGroup.
 *
 * This is built by MeshGroupGPUData::rebuild_from_meshes() and used by
 * the renderer to issue draw calls. It records:
 *  - which Mesh / submesh the draw originated from (for debugging/picking),
 *  - an index range inside the aggregated index buffer,
 *  - a Material pointer (and optionally an integer material index used
 *    by a MaterialManager / GPU SSBO).
 */
struct GroupSubmeshDraw {
    /// CPU mesh that owns the original submesh.
    std::shared_ptr<Mesh> mesh;

    /// Index of the submesh within MeshCPUData::submeshes of @ref mesh.
    std::size_t submesh_index{0};

    /// Offset into the aggregated index buffer (in index elements, not bytes).
    std::uint32_t index_offset{0};

    /// Number of indices to draw for this submesh.
    std::uint32_t index_count{0};

    /// Material assigned to this submesh (may be nullptr).
    std::shared_ptr<Material> material;

    /// Optional GPU-side material index (e.g. into a Material SSBO).
    std::uint32_t material_index{0};
};

/**
 * @brief Aggregated GPU buffers for a mesh group / chunk.
 *
 * A MeshGroupGPUData owns exactly one VAO and a small set of VBO/EBO
 * buffers that contain the geometry of *all* meshes in a MeshGroup.
 *
 * The layout is intentionally simple:
 *  - Attribute 0: vec3 position
 *  - Attribute 1: vec3 normal (optional)
 *  - Attribute 2: vec2 texcoord (optional)
 *
 * The @ref draws_ list stores one entry per Submesh across all meshes,
 * which the renderer can then turn into glDrawElements or multi-draw
 * indirect commands.
 */
class MeshGroupGPUData {
  public:
    using SPtr = std::shared_ptr<MeshGroupGPUData>;

    MeshGroupGPUData() = default;

    /**
     * @brief Rebuilds all GPU buffers from the provided meshes.
     *
     * Steps:
     *  1. Releases any previously allocated GPU buffers.
     *  2. Flattens vertex attributes from each MeshCPUData into large arrays.
     *  3. Offsets indices for each mesh and appends them to a unified index buffer.
     *  4. Uploads these arrays into a single VAO/VBO/EBO set.
     *  5. Fills @ref draws_ with one entry per Submesh across all meshes.
     *
     * All meshes are expected to be in RAM before calling this method
     * (i.e. Mesh::cpu_data() must be non-null and is_valid() must be true).
     *
     * @param meshes List of meshes whose CPU data will be aggregated.
     * @return true on success, false if no valid data was found.
     */
    bool rebuild_from_meshes(const std::vector<std::shared_ptr<Mesh>>& meshes);

    /**
     * @brief Releases all GPU buffers and clears the draw list.
     */
    void release_gpu();

    /// Returns the internal VAO used for rendering.
    VAOData* vao() const { return vao_.get(); }

    /// Returns the list of draws describing all submeshes in the group.
    const std::vector<GroupSubmeshDraw>& draws() const { return draws_; }

  private:
    std::unique_ptr<VAOData> vao_;
    std::unique_ptr<VBOData> position_vbo_;
    std::unique_ptr<VBOData> normal_vbo_;
    std::unique_ptr<VBOData> texcoord_vbo_;
    std::unique_ptr<VBOData> index_vbo_;

    std::vector<GroupSubmeshDraw> draws_;
};
