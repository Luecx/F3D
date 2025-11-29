#pragma once

#include <ecs.h>

#include <memory>
#include <utility>

#include "../../resources/mesh_data.h"

enum class TransparencyMode { Auto, ForceOpaque, ForceTransparent };

struct ModelComponent : ecs::ComponentOf<ModelComponent> {
    ModelComponent() = default;
    explicit ModelComponent(std::shared_ptr<MeshData> mesh_data, bool allow_instancing = true,
                            bool casts_shadows = true, bool double_sided = false,
                            TransparencyMode transparency = TransparencyMode::Auto)
        : mesh(std::move(mesh_data)), casts_shadows(casts_shadows), double_sided(double_sided),
          allow_instancing(allow_instancing), transparency_mode(transparency) {}

    [[nodiscard]] bool valid() const { return mesh != nullptr; }

    std::shared_ptr<MeshData> mesh;
    bool casts_shadows{true};
    bool double_sided{false};
    bool allow_instancing{true};
    TransparencyMode transparency_mode{TransparencyMode::Auto};
};

using ModelRef = ModelComponent;
