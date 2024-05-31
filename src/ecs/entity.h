#ifndef ECS_ECS_ENTITY_H_
#define ECS_ECS_ENTITY_H_

#include "hash.h"
#include "types.h"

#include <unordered_map>
#include <memory>

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
    //Container for the entity's components.
    std::unordered_map<Hash, ComponentContainerPtr> components;
    // Pointer to the ECS that owns this entity.
    ECS* ecs;
    // checks if its active or inactive.
    bool m_active = false;

    // Grant ECS access to private members.
    friend ECS;

    public:
    /**
     * @brief Constructs an Entity with a given ECS.
     *
     * @param p_ecs Pointer to the ECS that manages this entity.
     */
    Entity(ECS* p_ecs) : entity_id(INVALID_ID), ecs(p_ecs) {}

    /**
     * @brief Destructor for the Entity. Removes all components.
     */
    virtual ~Entity() {
        remove_all();
    }

    /**
     * @brief Checks if the entity has a component of the specified type.
     *
     * @tparam T The type of the component to check for.
     * @return true if the entity has the component, false otherwise.
     */
    template<typename T>
    bool has() const {
        Hash hash = get_type_hash<T>();
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
     * @return Reference to the component container of the specified type.
     */
    template<typename T>
    ComponentContainer<T>& get() {
        static ComponentContainer<T> emptyContainer {};
        if (entity_id == INVALID_ID)
            return emptyContainer;
        Hash hash = get_type_hash<T>();
        if (components.find(hash) != components.end()) {
            return *reinterpret_cast<ComponentContainer<T>*>(components.at(hash));
        }
        return emptyContainer;
    }

    /**
     * @brief Assigns a component to the entity.
     *
     * @tparam T The type of the component to assign.
     * @tparam Args The arguments to construct the component.
     * @param args The arguments to forward to the component's constructor.
     */
    template<typename T, typename... Args>
    void assign(Args&&... args);

    /**
     * @brief Removes a component of the specified type from the entity.
     *
     * @tparam T The type of the component to remove.
     */
    template<typename T>
    void remove();

    /**
     * @brief Removes all components from the entity.
     */
    void remove_all();

    /**
     * @brief Destroys the entity, removing all components and invalidating the entity.
     */
    void destroy();

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
    void activate();

    /**
     * @brief Deactivates the entity.
     */
    void deactivate();

    /**
     * @brief Sets the active state of the entity.
     *
     * @param val The new active state.
     */
    void set_active(bool val);

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

    // stream output to display hash, id, valid, active all nicely in new rows indented
    friend std::ostream& operator<<(std::ostream& os, const Entity& entity) {
            os << "Entity ID: " << entity.entity_id << std::endl;
            os << "\tValid: " << entity.valid() << std::endl;
            os << "\tActive: " << entity.active() << std::endl;
            return os;
    }

};

using EntityPtr = std::shared_ptr<Entity>;

} // namespace ecs

#endif // ECS_ECS_ENTITY_H_
