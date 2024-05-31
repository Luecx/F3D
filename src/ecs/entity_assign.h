//
// Created by Luecx on 16.06.2021.
//

#ifndef ECS_ECS_ASSIGN_H_
#define ECS_ECS_ASSIGN_H_

#include "ecs.h"
#include "entity.h"

namespace ecs {

/**
 * @brief Assigns a component of the specified type to the entity.
 *
 * This function assigns a new component to the entity. If the component already
 * exists, it updates the existing component with the new values. Otherwise, it
 * creates a new component and adds it to the entity.
 *
 * @tparam T The type of the component to assign.
 * @tparam Args The argument types to construct the component.
 * @param args The arguments to forward to the component's constructor.
 */
template<typename T, typename... Args>
inline void Entity::assign(Args&&... args) {
    T t {std::forward<Args>(args)...};

    if (has<T>()) {
        ComponentContainer<T> container {t};
        Hash hash = get_type_hash<T>();
        reinterpret_cast<ComponentContainer<T>*>(components.at(hash))->get() = t;
    } else {
        auto* container = new ComponentContainer<T> {t};
        Hash hash = get_type_hash<T>();
        components[hash] = container;
        ecs->component_added<T>(this);
    }
}

/**
 * @brief Removes a component of the specified type from the entity.
 *
 * This function removes a component of the given type from the entity if it exists.
 *
 * @tparam T The type of the component to remove.
 */
template<typename T>
inline void Entity::remove() {
    if (!has<T>()) return;

    Hash hash = get_type_hash<T>();
    auto* container = components.at(hash);
    // Optionally delete the container if dynamically allocated
    // delete container;
    components.erase(hash);
    ecs->component_removed<T>(this);
}

/**
 * @brief Removes all components from the entity.
 *
 * This function removes all components from the entity.
 */
inline void Entity::remove_all() {
    for (auto& pair : components) {
        ecs->component_removed(pair.first, this);
    }

    components.clear();
}

/**
 * @brief Destroys the entity.
 *
 * This function destroys the entity by removing it from the ECS and clearing all its components.
 */
inline void Entity::destroy() {
    // free all components
    remove_all();
    // remove entity from ecs
    ecs->destroy(this->entity_id);
}

/**
 * @brief Activates the entity.
 *
 * This function activates the entity, allowing it to be processed by systems.
 */
void Entity::activate() {
    if (!m_active) {
        m_active = true;
        ecs->entity_activated(this->entity_id);
    }
}

/**
 * @brief Deactivates the entity.
 *
 * This function deactivates the entity, preventing it from being processed by systems.
 */
void Entity::deactivate() {
    if (m_active) {
        m_active = false;
        ecs->entity_deactivated(this->entity_id);
    }
}

/**
 * @brief Sets the entity's active state.
 *
 * This function sets the entity's active state to the specified value.
 *
 * @param val The value to set the entity's active state to.
 */
void Entity::set_active(bool val) {
    if (val) {
        activate();
    } else {
        deactivate();
    }
}

}    // namespace ecs

#endif    // ECS_ECS_ASSIGN_H_
