#pragma once

#include <ecs.h>

#include <memory>
#include <utility>

#include "../../resources/mesh_data.h"

struct MeshRenderer : ecs::ComponentOf<MeshRenderer> {
    MeshRenderer() = default;
    explicit MeshRenderer(std::shared_ptr<MeshData> mesh_data) : mesh(std::move(mesh_data)) {}

    std::shared_ptr<MeshData> mesh;
    bool casts_shadows{true};
    bool double_sided{false};

    bool valid() const { return mesh != nullptr; }
};
