#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "InstanceBuffer.h"
#include "RenderScene.h"
#include "../resources/resource_types.h"
#include "../logging/logging.h"

struct InstanceDrawRange {
    GLuint base_instance{0};
    GLsizei instance_count{0};
};

struct MeshBatch {
    MeshData* mesh{nullptr};
    bool double_sided{false};
    std::vector<InstanceDrawRange> draws;
};

namespace detail {
inline std::uintptr_t make_batch_key(MeshData* mesh, bool double_sided) {
    return (reinterpret_cast<std::uintptr_t>(mesh) << 1) ^ (double_sided ? 0x1u : 0x0u);
}
} // namespace detail

template<typename Predicate>
std::vector<MeshBatch> build_mesh_batches(const RenderableList& renderables, InstanceBuffer& instance_buffer,
                                          Predicate&& predicate) {
    std::vector<MeshBatch> batches;
    std::unordered_map<std::uintptr_t, std::size_t> batch_lookup;

    for (const auto& renderable : renderables) {
        if (!predicate(renderable)) {
            continue;
        }
        auto* model = renderable.model;
        auto* instances = renderable.instances;
        if (!model || !instances || instances->count() == 0) {
            continue;
        }
        auto mesh_ptr = model->mesh.get();
        if (!mesh_ptr) {
            continue;
        }
        if (mesh_ptr->current_state() != resources::ResourceState::Gpu) {
            logging::log(0, logging::DEBUG, "RenderBatchBuilder skipping mesh not resident: " + mesh_ptr->get_path());
            continue;
        }
        auto base = instance_buffer.base_instance(instances);
        if (!base.has_value()) {
            logging::log(0, logging::WARNING,
                         "RenderBatchBuilder missing base instance for component " +
                             std::to_string(reinterpret_cast<std::uintptr_t>(instances)));
            continue;
        }

        auto key = detail::make_batch_key(mesh_ptr, model->double_sided);
        std::size_t index = 0;
        auto it = batch_lookup.find(key);
        if (it == batch_lookup.end()) {
            MeshBatch batch;
            batch.mesh = mesh_ptr;
            batch.double_sided = model->double_sided;
            index = batches.size();
            batches.push_back(std::move(batch));
            batch_lookup.emplace(key, index);
        } else {
            index = it->second;
        }

        auto& batch = batches[index];
        if (model->allow_instancing) {
            batch.draws.push_back(
                InstanceDrawRange{static_cast<GLuint>(*base), static_cast<GLsizei>(instances->count())});
        } else {
            for (std::size_t i = 0; i < instances->count(); ++i) {
                batch.draws.push_back(InstanceDrawRange{static_cast<GLuint>(*base + i), 1});
            }
        }
    }

    return batches;
}
