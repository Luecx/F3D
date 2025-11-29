#include "mesh_group_gpu_data.h"

#include "mesh.h"
#include "mesh_cpu_data.h"

#include <glad/glad.h>
#include <iostream>

bool MeshGroupGPUData::rebuild_from_meshes(const std::vector<std::shared_ptr<Mesh>>& meshes) {
    // Drop old GPU buffers and draw list.
    release_gpu();

    // ------------------------------------------------------------------
    // Aggregate CPU data across all meshes in the group.
    // ------------------------------------------------------------------
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<std::uint32_t> indices;

    bool any_normals   = false;
    bool any_texcoords = false;

    // First pass: detect if any mesh provides normals/texcoords.
    for (const auto& mesh : meshes) {
        if (!mesh) {
            continue;
        }
        auto cpu_ptr = mesh->cpu_data();
        if (!cpu_ptr) {
            continue;
        }
        const MeshCPUData& cpu = *cpu_ptr;
        if (!cpu.normals.empty()) {
            any_normals = true;
        }
        if (!cpu.texcoords.empty()) {
            any_texcoords = true;
        }
    }

    std::uint32_t vertex_base = 0;

    for (const auto& mesh : meshes) {
        if (!mesh) {
            continue;
        }
        auto cpu_ptr = mesh->cpu_data();
        if (!cpu_ptr) {
            continue;
        }

        const MeshCPUData& cpu = *cpu_ptr;
        if (!cpu.is_valid()) {
            continue;
        }

        const std::size_t v_count = cpu.vertex_count();
        const std::size_t idx_count_before = indices.size();

        // Append positions.
        positions.insert(positions.end(), cpu.positions.begin(), cpu.positions.end());

        // Append normals or zero-fill if normals are missing for this mesh.
        if (any_normals) {
            if (!cpu.normals.empty()) {
                normals.insert(normals.end(), cpu.normals.begin(), cpu.normals.end());
            } else {
                normals.resize(normals.size() + v_count * 3, 0.0f);
            }
        }

        // Append texcoords or zero-fill if texcoords are missing.
        if (any_texcoords) {
            if (!cpu.texcoords.empty()) {
                texcoords.insert(texcoords.end(), cpu.texcoords.begin(), cpu.texcoords.end());
            } else {
                texcoords.resize(texcoords.size() + v_count * 2, 0.0f);
            }
        }

        // Copy and offset indices.
        for (auto idx : cpu.indices) {
            indices.push_back(idx + vertex_base);
        }

        // Create draw entries for each submesh.
        for (std::size_t si = 0; si < cpu.submeshes.size(); ++si) {
            const Submesh& sm = cpu.submeshes[si];
            GroupSubmeshDraw entry;
            entry.mesh          = mesh;
            entry.submesh_index = si;
            entry.material      = sm.material;
            entry.index_offset  = static_cast<std::uint32_t>(idx_count_before + sm.index_offset);
            entry.index_count   = static_cast<std::uint32_t>(sm.index_count);
            entry.material_index = 0; // to be filled by a MaterialManager if desired
            draws_.push_back(entry);
        }

        vertex_base += static_cast<std::uint32_t>(v_count);
    }

    if (positions.empty() || indices.empty()) {
        // Nothing to upload.
        return false;
    }

    // ------------------------------------------------------------------
    // Upload to GPU.
    // ------------------------------------------------------------------
    vao_          = std::make_unique<VAOData>();
    position_vbo_ = std::make_unique<VBOData>();
    index_vbo_    = std::make_unique<VBOData>();

    vao_->bind();

    // Attribute 0: positions (vec3)
    glEnableVertexAttribArray(0);
    position_vbo_->store_data(0, 3, positions);

    // Attribute 1: normals (vec3), if any mesh provided them.
    if (any_normals) {
        normal_vbo_ = std::make_unique<VBOData>();
        glEnableVertexAttribArray(1);
        normal_vbo_->store_data(1, 3, normals);
    }

    // Attribute 2: texcoords (vec2), if any mesh provided them.
    if (any_texcoords) {
        texcoord_vbo_ = std::make_unique<VBOData>();
        glEnableVertexAttribArray(2);
        texcoord_vbo_->store_data(2, 2, texcoords);
    }

    // Index buffer
    index_vbo_->store_indices(indices);

    vao_->unbind();
    return true;
}

void MeshGroupGPUData::release_gpu() {
    index_vbo_.reset();
    texcoord_vbo_.reset();
    normal_vbo_.reset();
    position_vbo_.reset();
    vao_.reset();
    draws_.clear();
}
