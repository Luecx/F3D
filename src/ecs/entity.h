#ifndef ECS_ECS_ENTITY_H_
#define ECS_ECS_ENTITY_H_

#include "component.h"
#include "ecs_base.h"
#include "hash.h"
#include "types.h"

#include <memory>
#include <unordered_map>

namespace ecs {

/**
 * @brief Represents an entity in the ECS (Entity Component System).
 *
 * The Entity class manages a collection of components and provides methods
 * to add, remove, and access these components. It also supports basic operations
 * like checking for component existence and entity comparison.
 */
struct Entity {
    private:
    // Unique identifier for the entity.
    ID entity_id;
    // Container for the entity's components.
    std::unordered_map<Hash, ComponentPtr> components;
    // Pointer to the ECS that owns this entity.
    ECSBase* ecs;
    // Checks if its active or inactive.
    bool m_active = false;

    // Grant ECS access to private members.
    friend ECS;

    public:
    /**
     * @brief Constructs an Entity with a given ECS.
     *
     * @param p_ecs Pointer to the ECS that manages this entity.
     */
    Entity(ECSBase* p_ecs)
        : entity_id(INVALID_ID)
        , ecs(p_ecs) {}

    /**
     * @brief Destructor for the Entity. Removes all components.
     */
    virtual ~Entity() {
        remove_all_components();
    }

    /**
     * @brief Checks if the entity has a component of the specified type.
     *
     * @tparam T The type of the component to check for.
     * @return true if the entity has the component, false otherwise.
     */
    template<typename T>
    bool has() const {
        Hash hash = T::hash();
        return components.find(hash) != components.end();
    }

    /**
     * @brief Checks if the entity has components of all the specified types.
     *
     * @tparam T The first type of the component to check for.
     * @tparam V The second type of the component to check for.
     * @tparam Types The remaining types of components to check for.
     * @return true if the entity has all the components, false otherwise.
     */
    template<typename T, typename V, typename... Types>
    bool has() const {
        return has<T>() && has<V, Types...>();
    }

    /**
     * @brief Gets a reference to the component of the specified type.
     *
     * @tparam T The type of the component to get.
     * @return Reference to the component of the specified type.
     */
    template<typename T>
    std::shared_ptr<T> get() {
        Hash hash = get_type_hash<T>();
        if (components.find(hash) != components.end()) {
            return std::static_pointer_cast<T>(components.at(hash));
        }
        return nullptr;
    }

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
    inline void assign(Args&&... args) {
        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        Hash hash      = T::hash();

        // If the component already exists, remove it first
        if (has<T>()) {
            remove<T>();
        }

        // Add the new component
        components[hash] = component;
        ecs->component_added(hash, this);

        // notify all other components that a new component was added
        for (auto& [hash, comp] : components) {
            // dont do it for itself
            if (hash == component->get_hash())
                continue;

            // call the other_component_added function
            comp->other_component_added(hash);
            component->other_component_added(hash);
        }

        // inform the component that it was added to an active entity
        if (m_active) {
            component->entity_activated();
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
    inline void remove() {
        if (!has<T>())
            return;

        Hash hash = get_type_hash<T>();
        components[hash]->component_removed();
        components.erase(hash);
        ecs->component_removed(hash, this);
    }

    /**
     * @brief Removes all components from the entity.
     *
     * This function removes all components from the entity.
     */
    inline void remove_all_components() {
        for (auto& pair : components) {
            ecs->component_removed(pair.first, this);
        }
        for (auto& pair : components) {
            pair.second->component_removed();
        }

        components.clear();
    }

    /**
     * @brief Destroys the entity.
     *
     * This function destroys the entity by removing it from the ECS and clearing all its components.
     */
    inline void destroy() {
        // Free all components
        remove_all_components();
        // Remove entity from ecs
        ecs->destroy(this->entity_id);
    }

    /**
     * @brief Gets the entity's unique identifier.
     *
     * @return The entity's unique identifier.
     */
    ID id() const {
        return entity_id;
    }

    /**
     * @brief Checks if the entity is valid.
     *
     * @return true if the entity is valid, false otherwise.
     */
    bool valid() const {
        return entity_id != INVALID_ID;
    }

    /**
     * @brief Checks if the entity is active.
     *
     * @return true if the entity is active, false otherwise.
     */
    bool active() const {
        return m_active;
    }

    /**
     * @brief Activates the entity.
     */
    void activate() {
        if (!m_active) {
            m_active = true;
            ecs->entity_activated(this->entity_id);
            for (auto& [hash, component] : components) {
                component->entity_activated();
            }
        }
    }

    /**
     * @brief Deactivates the entity.
     */
    void deactivate() {
        if (m_active) {
            m_active = false;
            ecs->entity_deactivated(this->entity_id);
            for (auto& [hash, component] : components) {
                component->entity_deactivated();
            }
        }
    }

    /**
     * @brief Sets the entity's active state.
     *
     * This function sets the entity's active state to the specified value.
     *
     * @param val The value to set the entity's active state to.
     */
    void set_active(bool val) {
        if (val) {
            activate();
        } else {
            deactivate();
        }
    }

    // Comparison operators for the Entity class.
    bool operator==(const Entity& rhs) const {
        return entity_id == rhs.entity_id;
    }
    bool operator!=(const Entity& rhs) const {
        return !(rhs == *this);
    }
    bool operator<(const Entity& rhs) const {
        return entity_id < rhs.entity_id;
    }
    bool operator>(const Entity& rhs) const {
        return rhs < *this;
    }
    bool operator<=(const Entity& rhs) const {
        return !(rhs < *this);
    }
    bool operator>=(const Entity& rhs) const {
        return !(*this < rhs);
    }

    // Stream output to display hash, id, valid, active all nicely in new rows indented
    friend std::ostream& operator<<(std::ostream& os, const Entity& entity) {
        os << "Entity ID: " << entity.entity_id << std::endl;
        os << "\tValid: " << entity.valid() << std::endl;
        os << "\tActive: " << entity.active() << std::endl;
        return os;
    }
};

using EntityPtr = std::shared_ptr<Entity>;

}    // namespace ecs

#endif    // ECS_ECS_ENTITY_H_
