#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "../resource/mesh/mesh.h"
#include "../resource/resource_types.h"
#include "mesh_group_gpu_data.h"

class MaterialManager;
class Material;

/**
 * @brief A group (chunk) of meshes with a single aggregated GPU buffer.
 *
 * CPU meshes live in resource/mesh. GPU aggregation, instance SSBO,
 * and draw metadata live here in model/.
 */
class MeshGroup {
  public:
    using SPtr = std::shared_ptr<MeshGroup>;

    MeshGroup() = default;

    void add_mesh(const Mesh::SPtr& mesh);
    void remove_mesh(const Mesh::SPtr& mesh);

    const std::vector<Mesh::SPtr>& meshes() const { return meshes_; }
    std::shared_ptr<MeshGroupGPUData> gpu_data() const { return gpu_data_; }

    /// Request a minimum state (Drive/Ram/Gpu) for the group and its materials/textures.
    void request(resources::ResourceState state,
                       const std::function<std::uint32_t(const std::shared_ptr<Material>&)>& resolve_material_id);

    /// Release a state; resources may be freed if refcounts drop to zero.
    void release(resources::ResourceState state);

    bool has_gpu() const { return has_gpu_; }
    bool has_ram() const { return has_ram_; }

  private:
    void ensure_material_list();

    std::vector<Mesh::SPtr> meshes_;
    std::vector<std::shared_ptr<Material>> materials_used_;
    std::shared_ptr<MeshGroupGPUData> gpu_data_;
    bool has_ram_{false};
    bool has_gpu_{false};
};
