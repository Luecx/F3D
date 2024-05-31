//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_COMPONENT_H_
#define ECS_ECS_COMPONENT_H_

#include "types.h"

#include <ostream>

namespace ecs {

/**
 * @brief Base struct for component containers.
 * This acts as a base type for all component containers, allowing for polymorphic storage.
 */
struct ComponentContainerBase {};

/**
 * @brief Template struct for component containers.
 * This template struct stores a component of any type and provides access to it.
 *
 * @tparam T The type of the component to be stored.
 */
template<typename T>
struct ComponentContainer : public ComponentContainerBase {
    T component;    ///< The component instance stored in the container.

    /**
     * @brief Constructs a ComponentContainer with a given component.
     *
     * @param p_component The component to be stored in the container.
     */
    explicit ComponentContainer(T& p_component)
        : component(p_component) {};

    /**
     * @brief Overloaded arrow operator to access the component's members.
     *
     * @return Pointer to the stored component.
     */
    T* operator->() {
        return &component;
    }

    /**
     * @brief Gets a reference to the stored component.
     *
     * @return Reference to the stored component.
     */
    T& get() {
        return component;
    }

    /**
     * @brief Conversion operator to allow conversion to a reference of the component type.
     *
     * @return Reference to the stored component.
     */
    explicit operator T&() {
        return component;
    }

    /**
     * @brief Overloaded stream insertion operator for the ComponentContainer.
     *
     * @param os The output stream.
     * @param container The container to be outputted.
     * @return The output stream with the component inserted.
     */
    friend std::ostream& operator<<(std::ostream& os, const ComponentContainer& container) {
        os << container.component;
        return os;
    }
};

// If COMPONENT_SMART_POINTER is defined, the components will be stored as shared pointers
#ifdef COMPONENT_SMART_POINTER
using ComponentContainerPtr = std::shared_ptr<ComponentContainerBase>;
#else
using ComponentContainerPtr = ComponentContainerBase*;
#endif

}    // namespace ecs

#endif    // ECS_ECS_COMPONENT_H_
