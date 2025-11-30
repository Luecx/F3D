#pragma once

#include <vector>

#include "../ecs/component.h"
#include "../ecs/ids.h"
#include "transform.h"

/**
 * @brief Component describing a set of instances to render.
 *
 * Holds transforms for each instance and a link to the MeshGroup entity
 * that provides the geometry.
 */
struct InstancesComponent : ecs::ComponentOf<InstancesComponent> {
    ecs::EntityID mesh_group_entity{ecs::INVALID_ID};
    std::vector<Transform> transforms;

    std::size_t instance_count() const { return transforms.size(); }
};
