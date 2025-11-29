/**
* @file component.h
 * @brief Component base class and CRTP helper.
 */

#pragma once

#include <memory>
#include "types.h"
#include "hash.h"
#include "ids.h"
#include "ecs_base.h"

namespace ecs {

/**
 * @brief Base class for all components attached to entities.
 */
struct ComponentBase {
    ComponentBase() = default;
    virtual ~ComponentBase() = default;

    ECS*        ecs                 = nullptr;
    ComponentID component_id        = ComponentID{};
    ID          component_entity_id = INVALID_ID;

    virtual void component_removed() {}
    virtual void entity_activated() {}
    virtual void entity_deactivated() {}

    virtual void other_component_added(Hash /*hash*/) {}
    virtual void other_component_removed(Hash /*hash*/) {}

    virtual Hash get_hash() const { return INVALID_HASH; }
};

/**
 * @brief CRTP helper to define a component with a static type hash.
 *
 * @tparam T Derived component type.
 */
template<typename T>
struct ComponentOf : public ComponentBase {

    static Hash hash() {
        return get_type_hash<T>();
    }

    Hash get_hash() const override {
        return hash();
    }
};

using ComponentPtr = std::unique_ptr<ComponentBase>;

} // namespace ecs
