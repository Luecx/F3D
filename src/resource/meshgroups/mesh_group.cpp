#include "mesh_group.h"

#include <algorithm>

#include "../meshes/mesh_cpu_data.h"

void MeshGroup::impl_load(ResourceState state) {
    switch (state) {
        case ResourceState::Cpu:
            // No CPU-side aggregation state; meshes themselves own CPU data.
            logging::log(reslog::MESH, logging::DEBUG,
                         "CPU acquire MeshGroup (no-op)");
            break;

        case ResourceState::Gpu:
            logging::log(reslog::MESH, logging::INFO,
                         "GPU acquire MeshGroup: building GPU buffers");
            build_gpu_buffers();
            break;

        case ResourceState::Drive:
        default:
            // Conceptual only.
            break;
    }
}

void MeshGroup::impl_unload(ResourceState state) {
    switch (state) {
        case ResourceState::Cpu:
            // Nothing to free on CPU.
            logging::log(reslog::MESH, logging::DEBUG,
                         "CPU release MeshGroup (no-op)");
            break;

        case ResourceState::Gpu:
            logging::log(reslog::MESH, logging::INFO,
                         "GPU release MeshGroup: destroying GPU buffers");
            destroy_gpu_buffers();
            break;

        case ResourceState::Drive:
        default:
            break;
    }
}

void MeshGroup::build_gpu_buffers() {
    destroy_gpu_buffers(); // in case this is a rebuild

    std::vector<float>          positions;
    std::vector<float>          normals;
    std::vector<float>          texcoords;
    std::vector<std::uint32_t>  indices;
    draws_.clear();

    // Collect valid meshes and detect whether the group has normals/texcoords.
    struct MeshEntry {
        std::shared_ptr<Mesh> mesh;
        const MeshCPUData*    cpu{nullptr};
    };

    std::vector<MeshEntry> entries;
    entries.reserve(meshes_.size());

    bool group_has_normals   = false;
    bool group_has_texcoords = false;

    for (auto& mesh : meshes_) {
        if (!mesh) continue;

        // Temporarily acquire CPU data for this build.
        mesh->require(ResourceState::Cpu);

        const MeshCPUData* cpu = mesh->cpu_data();
        if (!cpu || !cpu->is_valid()) {
            logging::log(reslog::MESH, logging::WARNING,
                         "MeshGroup: skipping invalid mesh '" + mesh->path() + "'");
            mesh->release(ResourceState::Cpu);
            continue;
        }

        if (!cpu->normals.empty())   group_has_normals   = true;
        if (!cpu->texcoords.empty()) group_has_texcoords = true;

        entries.push_back(MeshEntry{mesh, cpu});
    }

    if (entries.empty()) {
        logging::log(reslog::MESH, logging::ERROR,
                     "MeshGroup build found no valid meshes");
        return;
    }

    std::uint32_t base_vertex = 0;

    for (const auto& entry : entries) {
        const MeshCPUData* cpu     = entry.cpu;
        const std::uint32_t vcount = static_cast<std::uint32_t>(cpu->vertex_count());

        // Positions: always required
        positions.insert(positions.end(), cpu->positions.begin(), cpu->positions.end());

        // Normals: if the group uses normals, ensure each vertex gets one.
        if (group_has_normals) {
            if (!cpu->normals.empty()) {
                normals.insert(normals.end(), cpu->normals.begin(), cpu->normals.end());
            } else {
                // Fill with default normals (0,0,1) per vertex.
                for (std::uint32_t i = 0; i < vcount; ++i) {
                    normals.push_back(0.0f);
                    normals.push_back(0.0f);
                    normals.push_back(1.0f);
                }
            }
        }

        // Texcoords: if the group uses texcoords, ensure each vertex gets one.
        if (group_has_texcoords) {
            if (!cpu->texcoords.empty()) {
                texcoords.insert(texcoords.end(), cpu->texcoords.begin(), cpu->texcoords.end());
            } else {
                // Fill with zeros (0,0) per vertex.
                for (std::uint32_t i = 0; i < vcount; ++i) {
                    texcoords.push_back(0.0f);
                    texcoords.push_back(0.0f);
                }
            }
        }

        const std::size_t index_offset_before = indices.size();

        // Offset indices by base_vertex.
        for (auto idx : cpu->indices) {
            indices.push_back(idx + base_vertex);
        }

        // Create draw items for each submesh of this mesh.
        for (const auto& sm : cpu->submeshes) {
            MeshGroupDrawItem di;
            di.first_index = static_cast<std::uint32_t>(index_offset_before + sm.index_offset);
            di.index_count = static_cast<std::uint32_t>(sm.index_count);
            di.base_vertex = base_vertex;

            // Default material index = 0. If resolver exists and material is set,
            // ask resolver for an index into the global material table.
            di.material_index = 0;
            if (material_index_resolver_ && sm.material) {
                di.material_index = material_index_resolver_(sm.material);
            }

            draws_.push_back(di);
        }

        base_vertex += vcount;
    }

    if (positions.empty() || indices.empty()) {
        logging::log(reslog::MESH, logging::ERROR,
                     "MeshGroup build: no positions or indices after aggregation");
        // Release CPU users before returning.
        for (auto& e : entries) {
            e.mesh->release(ResourceState::Cpu);
        }
        return;
    }

    // ---- Create GPU buffers using VAOData/VBOData wrappers ----

    vao_ = std::make_shared<VAOData>();

    // Positions (attribute 0)
    vbo_positions_ = std::make_shared<VBOData>(GL_ARRAY_BUFFER);
    vbo_positions_->allocate(
        static_cast<GLsizeiptr>(sizeof(float) * positions.size()),
        positions.data(),
        GL_STATIC_DRAW);
    vao_->add_vbo(vbo_positions_);
    vao_->set_attribute(
        /*index=*/0,
        /*size=*/3,
        /*type=*/GL_FLOAT,
        /*normalized=*/GL_FALSE,
        /*stride=*/3 * static_cast<GLsizei>(sizeof(float)),
        /*offset=*/reinterpret_cast<void*>(0),
        /*vbo=*/vbo_positions_,
        /*integerAttribute=*/false);

    // Normals (attribute 1)
    if (group_has_normals) {
        vbo_normals_ = std::make_shared<VBOData>(GL_ARRAY_BUFFER);
        vbo_normals_->allocate(
            static_cast<GLsizeiptr>(sizeof(float) * normals.size()),
            normals.data(),
            GL_STATIC_DRAW);
        vao_->add_vbo(vbo_normals_);
        vao_->set_attribute(
            /*index=*/1,
            /*size=*/3,
            /*type=*/GL_FLOAT,
            /*normalized=*/GL_FALSE,
            /*stride=*/3 * static_cast<GLsizei>(sizeof(float)),
            /*offset=*/reinterpret_cast<void*>(0),
            /*vbo=*/vbo_normals_,
            /*integerAttribute=*/false);
    }

    // Texcoords (attribute 2)
    if (group_has_texcoords) {
        vbo_texcoords_ = std::make_shared<VBOData>(GL_ARRAY_BUFFER);
        vbo_texcoords_->allocate(
            static_cast<GLsizeiptr>(sizeof(float) * texcoords.size()),
            texcoords.data(),
            GL_STATIC_DRAW);
        vao_->add_vbo(vbo_texcoords_);
        vao_->set_attribute(
            /*index=*/2,
            /*size=*/2,
            /*type=*/GL_FLOAT,
            /*normalized=*/GL_FALSE,
            /*stride=*/2 * static_cast<GLsizei>(sizeof(float)),
            /*offset=*/reinterpret_cast<void*>(0),
            /*vbo=*/vbo_texcoords_,
            /*integerAttribute=*/false);
    }

    // Index buffer (EBO)
    ebo_ = std::make_shared<VBOData>(GL_ELEMENT_ARRAY_BUFFER);
    ebo_->allocate(
        static_cast<GLsizeiptr>(sizeof(std::uint32_t) * indices.size()),
        indices.data(),
        GL_STATIC_DRAW);

    // NOTE:
    // We do *not* rely on VAO storing the EBO binding here because VBOData::allocate()
    // binds + glBufferData + unbind. Instead, the renderer should bind ebo_ explicitly
    // before glDrawElements* calls:
    //
    //    group->vao()->bind();
    //    group->index_buffer()->bind();
    //    glDrawElements(...);
    //    group->index_buffer()->unbind();
    //    group->vao()->unbind();

    // Release CPU usage we acquired for the build.
    for (auto& e : entries) {
        e.mesh->release(ResourceState::Cpu);
    }

    logging::log(reslog::MESH, logging::INFO,
                 "MeshGroup GPU buffers built: verts=" +
                     std::to_string(positions.size() / 3) +
                     " indices=" + std::to_string(indices.size()) +
                     " draws=" + std::to_string(draws_.size()));
}

void MeshGroup::destroy_gpu_buffers() {
    ebo_.reset();
    vbo_positions_.reset();
    vbo_normals_.reset();
    vbo_texcoords_.reset();
    vao_.reset();
    draws_.clear();
}
