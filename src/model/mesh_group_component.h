#pragma once

#include <memory>

#include "../ecs/component.h"
#include "mesh_group.h"

/**
 * @brief Component that owns a mesh group resource.
 *
 * Typically attached to one entity representing the combined VAO/VBO
 * built from several meshes.
 */
struct MeshGroupComponent : ecs::ComponentOf<MeshGroupComponent> {
    std::shared_ptr<MeshGroup> group;
};
