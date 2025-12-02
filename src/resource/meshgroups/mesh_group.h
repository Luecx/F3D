#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <functional>

#include "../resource_base.h"
#include "../resource_state.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

#include "../../gldata/vao_data.h"
#include "../../gldata/vbo_data.h"

#include "../meshes/mesh.h"

class Material;

/**
 * @brief Draw item: a sub-range in the index buffer with base vertex and material index.
 *
 * CPU-side only. Used by the renderer to submit draw calls.
 *
 * material_index refers to an entry in a globally managed material table
 * (e.g., MaterialManager's SSBO).
 */
struct MeshGroupDrawItem {
    std::uint32_t first_index{0};
    std::uint32_t index_count{0};
    std::uint32_t base_vertex{0};
    std::uint32_t material_index{0};  ///< Index into global material SSBO.
};

/**
 * @brief GPU-only geometry bucket aggregating several Mesh instances.
 *
 * Responsibilities:
 *  - On impl_load(Gpu): gather CPU data from all attached Meshes and build:
 *      - one VAO
 *      - one VBO for positions
 *      - optional VBOs for normals/texcoords
 *      - one EBO (index buffer)
 *      - a list of MeshGroupDrawItem items
 *  - On impl_unload(Gpu): destroy all GPU buffers.
 *
 * MeshGroup itself does not own any CPU geometry; it only aggregates Mesh CPU
 * data into GPU buffers.
 *
 * Material mapping is done via an optional resolver callback that maps
 * std::shared_ptr<Material> -> material_index used in the draw items.
 */
class MeshGroup : public ResourceBase {
    public:
    using Ptr = std::shared_ptr<MeshGroup>;

    /// Attach a mesh to this group. Changes take effect on next GPU (re)build.
    void add_mesh(const std::shared_ptr<Mesh>& mesh) { meshes_.push_back(mesh); }

    /// Optional: clear all meshes (will require re-build on next GPU acquire).
    void clear_meshes() { meshes_.clear(); }

    /// Access draw items (valid after require(ResourceState::Gpu)).
    const std::vector<MeshGroupDrawItem>& draws() const { return draws_; }

    /// Access VAO (valid after require(ResourceState::Gpu)).
    VAOData* vao() const noexcept { return vao_.get(); }

    /// Access index buffer (bind before glDraw* with indices).
    VBOData* index_buffer() const noexcept { return ebo_.get(); }

    /**
     * @brief Set resolver used to map a Material pointer to a material_index.
     *
     * Typical usage: wire this to your MaterialManager, so that for each
     * submesh's Material, we get an integer index into the material SSBO.
     *
     * Example:
     *   group->set_material_index_resolver(
     *       [&](const std::shared_ptr<Material>& m) {
     *           return material_manager.get_or_assign_index(m);
     *       });
     */
    void set_material_index_resolver(
        std::function<std::uint32_t(const std::shared_ptr<Material>&)> resolver)
    {
        material_index_resolver_ = std::move(resolver);
    }

    protected:
    void impl_load(ResourceState state) override;
    void impl_unload(ResourceState state) override;

    private:
    void build_gpu_buffers();
    void destroy_gpu_buffers();

    std::vector<std::shared_ptr<Mesh>> meshes_;

    VAOData::SPtr  vao_;
    VBOData::SPtr  vbo_positions_;
    VBOData::SPtr  vbo_normals_;
    VBOData::SPtr  vbo_texcoords_;
    VBOData::SPtr  ebo_;

    std::vector<MeshGroupDrawItem> draws_;

    // Optional mapping from Material -> material_index in a global material table.
    std::function<std::uint32_t(const std::shared_ptr<Material>&)>
        material_index_resolver_;
};
