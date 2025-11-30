#include "mesh_group_gpu_data.h"

#include "../resource/mesh/mesh.h"
#include "../resource/mesh/mesh_cpu_data.h"
#include "../resource/resource_logging.h"
#include "../logging/logging.h"

#include "../glad.h"

bool MeshGroupGPUData::rebuild_from_meshes(
    const std::vector<std::shared_ptr<Mesh>>& meshes,
    const std::function<std::uint32_t(const std::shared_ptr<Material>&)>& resolve_material_id) {
    release_gpu();
    logging::log(reslog::MESH, logging::INFO, "Rebuilding MeshGroup GPU buffers");

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<std::uint32_t> indices;

    bool any_normals   = false;
    bool any_texcoords = false;

    for (const auto& mesh : meshes) {
        if (!mesh) continue;
        auto cpu_ptr = mesh->cpu_data();
        if (!cpu_ptr) continue;
        const MeshCPUData& cpu = *cpu_ptr;
        if (!cpu.normals.empty()) any_normals = true;
        if (!cpu.texcoords.empty()) any_texcoords = true;
    }

    std::uint32_t vertex_base = 0;
    std::vector<SubmeshInfoGPU> gpu_submesh_infos;

    for (const auto& mesh : meshes) {
        if (!mesh) continue;
        auto cpu_ptr = mesh->cpu_data();
        if (!cpu_ptr) continue;
        const MeshCPUData& cpu = *cpu_ptr;
        if (!cpu.is_valid()) continue;

        const std::size_t v_count = cpu.vertex_count();
        const std::size_t idx_count_before = indices.size();

        positions.insert(positions.end(), cpu.positions.begin(), cpu.positions.end());

        if (any_normals) {
            if (!cpu.normals.empty()) {
                normals.insert(normals.end(), cpu.normals.begin(), cpu.normals.end());
            } else {
                normals.resize(normals.size() + v_count * 3, 0.0f);
            }
        }

        if (any_texcoords) {
            if (!cpu.texcoords.empty()) {
                texcoords.insert(texcoords.end(), cpu.texcoords.begin(), cpu.texcoords.end());
            } else {
                texcoords.resize(texcoords.size() + v_count * 2, 0.0f);
            }
        }

        for (auto idx : cpu.indices) {
            indices.push_back(idx + vertex_base);
        }

        for (std::size_t si = 0; si < cpu.submeshes.size(); ++si) {
            const Submesh& sm = cpu.submeshes[si];
            GroupSubmeshDraw entry;
            entry.mesh           = mesh;
            entry.submesh_index  = si;
            entry.material       = sm.material;
            entry.index_offset   = static_cast<std::uint32_t>(idx_count_before + sm.index_offset);
            entry.index_count    = static_cast<std::uint32_t>(sm.index_count);
            entry.material_index = resolve_material_id ? resolve_material_id(sm.material) : 0;
            logging::log(reslog::MESH, logging::DEBUG,
                         "Submesh material id resolved to " + std::to_string(entry.material_index));
            draws_.push_back(entry);

            SubmeshInfoGPU info{};
            info.material_id = entry.material_index;
            gpu_submesh_infos.push_back(info);
        }

        vertex_base += static_cast<std::uint32_t>(v_count);
    }

    if (positions.empty() || indices.empty()) {
        logging::log(reslog::MESH, logging::ERROR, "MeshGroup rebuild found no geometry");
        return false;
    }

    vao_          = std::make_unique<VAOData>();
    position_vbo_ = std::make_unique<VBOData>();
    index_vbo_    = std::make_unique<VBOData>();

    vao_->bind();

    glEnableVertexAttribArray(0);
    position_vbo_->store_data(0, 3, positions);

    if (any_normals) {
        normal_vbo_ = std::make_unique<VBOData>();
        glEnableVertexAttribArray(1);
        normal_vbo_->store_data(1, 3, normals);
    }

    if (any_texcoords) {
        texcoord_vbo_ = std::make_unique<VBOData>();
        glEnableVertexAttribArray(2);
        texcoord_vbo_->store_data(2, 2, texcoords);
    }

    index_vbo_->store_indices(indices);
    vao_->unbind();

    // Upload per-draw material ids.
    submesh_info_ssbo_ = std::make_unique<SSBOData>();
    if (!gpu_submesh_infos.empty()) {
        submesh_info_ssbo_->update_data(
            static_cast<GLsizeiptr>(gpu_submesh_infos.size() * sizeof(SubmeshInfoGPU)),
            gpu_submesh_infos.data());
    }

    logging::log(reslog::MESH, logging::INFO, "MeshGroup rebuild complete with " +
                                             std::to_string(draws_.size()) + " submesh draws");
    return true;
}

void MeshGroupGPUData::upload_instances(std::size_t instance_count) {
    if (!instance_ssbo_) {
        instance_ssbo_ = std::make_unique<SSBOData>();
    }

    std::vector<InstanceData> payload(instance_count);
    // With Transform currently empty, fill identity matrices.
    for (auto& inst : payload) {
        inst.model[0]  = 1.0f;
        inst.model[5]  = 1.0f;
        inst.model[10] = 1.0f;
        inst.model[15] = 1.0f;
    }

    if (!payload.empty()) {
        instance_ssbo_->update_data(
            static_cast<GLsizeiptr>(payload.size() * sizeof(InstanceData)),
            payload.data());
    }
}

void MeshGroupGPUData::release_gpu() {
    submesh_info_ssbo_.reset();
    instance_ssbo_.reset();
    index_vbo_.reset();
    texcoord_vbo_.reset();
    normal_vbo_.reset();
    position_vbo_.reset();
    vao_.reset();
    draws_.clear();
}
