//
// Created by Finn Eggers on 31.05.24.
//

#ifndef F3D_ECS_BASE_H
#define F3D_ECS_BASE_H

#include "types.h"

namespace ecs{

struct ECSBase {

    /**
     * @brief Template function to handle the removal of a specific component type from an entity.
     *
     * @tparam C The type of the component.
     * @param entity The entity from which the component is removed.
     */
    virtual void component_removed(Hash hash, Entity* entity) = 0;

    /**
     * @brief Template function to handle the addition of a specific component type to an entity.
     *
     * @tparam C The type of the component.
     * @param entity The entity to which the component is added.
     */
    virtual void component_added(Hash hash, Entity* entity) = 0;

    /**
     * @brief Adds an entity to the ECS.
     * @param entity_id
     */
    virtual void entity_activated(ID entity_id) = 0;

    /**
     * @brief Removes an entity from the ECS.
     * @param entity_id
     */
    virtual void entity_deactivated(ID entity_id) = 0;

    /**
     * @brief Destroys an entity.
     * @param id
     */
    virtual void destroy(ID id) = 0;
};

}

#endif    // F3D_ECS_BASE_H
