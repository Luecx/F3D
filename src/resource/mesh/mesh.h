#pragma once

#include <memory>
#include <string>

#include "../resource_base.h"
#include "../resource_state.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

#include "mesh_cpu_data.h"

class TextureManager;
class MaterialManager;

/**
 * @brief CPU-only mesh resource.
 *
 * Responsibilities:
 *  - Load geometry and submesh/material info from disk (e.g. OBJ + MTL).
 *  - Provide access to MeshCPUData.
 *
 * Mesh has **no GPU representation** itself. GPU buffers are built by MeshGroup.
 */
class Mesh : public ResourceBase, public std::enable_shared_from_this<Mesh> {
    public:
    using Ptr = std::shared_ptr<Mesh>;

    Mesh(std::string path,
         TextureManager* tex_mgr   = nullptr,
         MaterialManager* mat_mgr  = nullptr);

    const std::string& path() const noexcept { return path_; }

    const MeshCPUData* cpu_data() const noexcept { return cpu_data_.get(); }
    MeshCPUData*       cpu_data()       noexcept { return cpu_data_.get(); }

    protected:
    void impl_load(ResourceState state) override;
    void impl_unload(ResourceState state) override;

    private:
    void load_cpu();    // load from file into MeshCPUData
    void unload_cpu();  // free MeshCPUData

    void load_obj();    // internal: parse OBJ/MTL

    std::string path_;
    std::unique_ptr<MeshCPUData> cpu_data_;

    TextureManager*  tex_mgr_{nullptr};
    MaterialManager* mat_mgr_{nullptr};
};
