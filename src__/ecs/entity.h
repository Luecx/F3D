/**
 * @file entity.h
 * @brief Entity type holding a set of components.
 */

#pragma once

#include <unordered_map>
#include <memory>
#include <ostream>

#include "types.h"
#include "ids.h"
#include "component.h"
#include "ecs_base.h"

namespace ecs {

struct ECS;
struct ComponentEntityList;

/**
 * @brief Runtime representation of an entity, holding its components.
 */
struct Entity {
private:
    EntityID entity_id{};
    std::unordered_map<Hash, ComponentPtr> components{};
    ECSBase* ecs = nullptr;
    bool m_active = false;

    friend ECS;
    friend ComponentEntityList;

public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(ECSBase* p_ecs = nullptr)
        : entity_id{INVALID_ID}, ecs(p_ecs) {}

    Entity(Entity&& other) noexcept
        : entity_id(other.entity_id)
        , components(std::move(other.components))
        , ecs(other.ecs)
        , m_active(other.m_active)
    {
        other.entity_id.id = INVALID_ID;
        other.ecs          = nullptr;
        other.m_active     = false;
    }

    Entity& operator=(Entity&& other) noexcept {
        if (this == &other) return *this;

        remove_all_components();

        entity_id = other.entity_id;
        components = std::move(other.components);
        ecs = other.ecs;
        m_active = other.m_active;

        other.entity_id.id = INVALID_ID;
        other.ecs          = nullptr;
        other.m_active     = false;
        return *this;
    }

    virtual ~Entity() {
        remove_all_components();
    }

    // ---- component access ----

    template<typename T>
    bool has() const {
        Hash hash = T::hash();
        return components.find(hash) != components.end();
    }

    template<typename T, typename V, typename... Types>
    bool has() const {
        return has<T>() && has<V, Types...>();
    }

    template<typename T>
    T* get() {
        Hash hash = get_type_hash<T>();
        auto it = components.find(hash);
        if (it != components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T, typename... Args>
    ComponentID assign(Args&&... args) {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        Hash hashing   = T::hash();

        component->ecs = reinterpret_cast<ECS*>(ecs);
        component->component_id = ComponentID{entity_id, hashing};

        if (has<T>()) {
            remove_component<T>();
        }

        components[hashing] = std::move(component);
        ecs->component_added(hashing, id());

        // notify others
        for (auto& [hash, comp_ptr] : components) {
            if (hash == hashing) continue;
            if (comp_ptr) {
                comp_ptr->other_component_added(hashing);
                components[hashing]->other_component_added(hash);
            }
        }

        auto* new_component = components[hashing].get();

        if (m_active && new_component) {
            new_component->entity_activated();
        }

        return new_component ? new_component->component_id : ComponentID{};
    }

    template<typename T>
    void remove_component() {
        Hash hash = get_type_hash<T>();
        auto it = components.find(hash);
        if (it == components.end()) return;

        it->second->component_removed();
        ecs->component_removed(hash, id());
        components.erase(it);
    }

    void remove_all_components() {
        for (auto& pair : components) {
            ecs->component_removed(pair.first, id());
        }
        for (auto& pair : components) {
            pair.second->component_removed();
        }
        components.clear();
    }

    // ---- id / state ----

    EntityID id() const { return entity_id; }

    bool valid() const { return entity_id.id != INVALID_ID; }

    bool active() const { return m_active; }

    void activate() {
        if (!m_active) {
            m_active = true;
            ecs->entity_activated(entity_id);
            for (auto& [_, component] : components) {
                component->entity_activated();
            }
        }
    }

    void deactivate() {
        if (m_active) {
            m_active = false;
            ecs->entity_deactivated(entity_id);
            for (auto& [_, component] : components) {
                component->entity_deactivated();
            }
        }
    }

    void set_active(bool val) {
        val ? activate() : deactivate();
    }

    void destroy() {
        ecs->destroy_entity(entity_id);
    }

    // ---- comparisons / debug ----

    bool operator==(const Entity& rhs) const { return entity_id.id == rhs.entity_id.id; }
    bool operator!=(const Entity& rhs) const { return !(*this == rhs); }
    bool operator<(const Entity& rhs)  const { return entity_id.id < rhs.entity_id.id; }
    bool operator>(const Entity& rhs)  const { return rhs < *this; }
    bool operator<=(const Entity& rhs) const { return !(rhs < *this); }
    bool operator>=(const Entity& rhs) const { return !(*this < rhs); }

    friend std::ostream& operator<<(std::ostream& os, const Entity& entity) {
        os << "Entity ID: " << entity.entity_id.id << '\n';
        os << "\tValid: "  << (entity.valid()  ? "true" : "false") << '\n';
        os << "\tActive: " << (entity.active() ? "true" : "false") << '\n';
        return os;
    }
};

} // namespace ecs
