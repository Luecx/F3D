#pragma once

#include "resource_data.h"

#include "../gldata/vao_data.h"
#include "../gldata/vbo_data.h"
#include "../math/mat.h"
#include "material_data.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct MeshGeometry {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<uint32_t> indices;
    std::vector<int> material_slots;
};

struct MeshGpuBuffers {
    VAOData::UPtr vao;
    VBOData::UPtr position_vbo;
    VBOData::UPtr normal_vbo;
    VBOData::UPtr uv_vbo;
    VBOData::UPtr material_vbo;
    VBOData::UPtr index_vbo;
};

struct MeshData : public ResourceData {
    explicit MeshData(std::string path);

    bool load_to_ram() override;
    void unload_from_ram() override;
    bool load_to_gpu() override;
    void unload_from_gpu() override;

    const MeshGeometry& geometry() const { return geometry_; }
    const MeshGpuBuffers& gpu_buffers() const { return gpu_; }
    std::size_t vertex_count() const { return geometry_.positions.size() / 3; }
    std::size_t index_count() const { return geometry_.indices.size(); }

    void draw() const;
    void draw_instanced(GLsizei instance_count, GLuint base_instance = 0) const;

    bool has_transparent_materials() const { return has_transparent_materials_; }
    bool has_opaque_materials() const { return has_opaque_materials_; }

  private:
    MeshGeometry geometry_;
    MeshGpuBuffers gpu_;
    std::vector<std::shared_ptr<MaterialData>> material_slots_;
    bool has_transparent_materials_{false};
    bool has_opaque_materials_{false};
};
