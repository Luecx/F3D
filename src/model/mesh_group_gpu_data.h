#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "../resource/mesh/submesh.h"
#include "../gldata/vao_data.h"
#include "../gldata/vbo_data.h"
#include "../gldata/ssbo_data.h"

class Mesh;
struct MeshCPUData;
class Material;

/**
 * @brief GPU-side metadata per submesh draw.
 */
struct GroupSubmeshDraw {
    std::shared_ptr<Mesh> mesh;
    std::size_t           submesh_index{0};
    std::uint32_t         index_offset{0};
    std::uint32_t         index_count{0};
    std::shared_ptr<Material> material;
    std::uint32_t         material_index{0};
};

/**
 * @brief Per-instance payload uploaded into an SSBO.
 *
 * For now this holds a model matrix (row-major) and an entity id.
 */
struct InstanceData {
    float        model[16]{};
    std::uint32_t entity_id{0};
    std::uint32_t padding0{0};
    std::uint32_t padding1{0};
    std::uint32_t padding2{0};
};

/**
 * @brief SSBO entry describing material id for each draw in the group.
 */
struct SubmeshInfoGPU {
    std::uint32_t material_id{0};
    std::uint32_t pad0{0};
    std::uint32_t pad1{0};
    std::uint32_t pad2{0};
};

/**
 * @brief Aggregated GPU buffers for a mesh group / chunk.
 *
 * Owns one VAO/VBO/EBO for geometry, one SSBO for per-instance data,
 * and one SSBO for per-draw material ids (aligned with draws()).
 */
class MeshGroupGPUData {
  public:
    using SPtr = std::shared_ptr<MeshGroupGPUData>;

    MeshGroupGPUData() = default;

    bool rebuild_from_meshes(
        const std::vector<std::shared_ptr<Mesh>>& meshes,
        const std::function<std::uint32_t(const std::shared_ptr<Material>&)>& resolve_material_id);

    void release_gpu();

    VAOData* vao() const { return vao_.get(); }
    const std::vector<GroupSubmeshDraw>& draws() const { return draws_; }

    /**
     * @brief Uploads per-instance data into the instance SSBO.
     *
     * With Transform currently empty, this uploads identity matrices.
     */
    void upload_instances(std::size_t instance_count);

  private:
    std::unique_ptr<VAOData> vao_;
    std::unique_ptr<VBOData> position_vbo_;
    std::unique_ptr<VBOData> normal_vbo_;
    std::unique_ptr<VBOData> texcoord_vbo_;
    std::unique_ptr<VBOData> index_vbo_;

    std::unique_ptr<SSBOData> instance_ssbo_;
    std::unique_ptr<SSBOData> submesh_info_ssbo_;

    std::vector<GroupSubmeshDraw> draws_;
};
