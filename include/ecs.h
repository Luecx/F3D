

// begin --- event.h --- 

//
// Created by Luecx on 16.06.2021.
//

#ifndef ECS_ECS_EVENT_H_
#define ECS_ECS_EVENT_H_

// begin --- hash.h --- 

//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_HASH_H_
#define ECS_ECS_HASH_H_

// begin --- types.h --- 

//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_TYPES_H_
#define ECS_ECS_TYPES_H_

#include <cstdint>
#include <typeindex>

namespace ecs {

struct Entity;

struct ECSBase;
struct ECS;

struct ComponentEntityList;

struct System;

template<typename... RTypes>
struct EntityIterator;

template<typename... RTypes>
struct EntitySubSet;

typedef std::size_t     ID;
typedef std::type_index Hash;

#define INVALID_ID   ID(-1)
#define INVALID_HASH Hash(typeid(void))


}    // namespace ecs_

#endif    // ECS_ECS_TYPES_H_


// end --- types.h --- 



namespace ecs {

/**
 * @brief Generates a hash for the specified type.
 *
 * This function template generates a hash value for a given type using
 * the C++ type_index and typeid mechanisms. This is useful for type identification
 * and comparison within the ECS (Entity Component System) framework.
 *
 * @tparam T The type for which the hash is to be generated.
 * @return Hash The generated hash value representing the type.
 */
template<typename T>
Hash get_type_hash() {
    return std::type_index(typeid(T));
}

}    // namespace ecs_

#endif    // ECS_ECS_HASH_H_


// end --- hash.h --- 



// begin --- ids.h --- 

//
// Created by Finn Eggers on 09.06.24.
//

#ifndef ECS_IDS_H
#define ECS_IDS_H

namespace ecs {

struct EntityID {
    ID id = INVALID_ID;
    operator ID() const { return id; }
    operator ID&() { return id; }
};

struct ComponentID {
    ID  id    = INVALID_ID;
    Hash hash = INVALID_HASH;
    operator ID() const { return id; }
    operator ID&() { return id; }
    operator Hash() const { return hash; }
    operator Hash&() { return hash; }
};

struct SystemID {
    ID id = INVALID_ID;
    operator ID() const { return id; }
    operator ID&() { return id; }
};

struct EventListenerID {
    ID   id   = INVALID_ID;
    Hash hash = INVALID_HASH;
    operator ID() const { return id; }
    operator ID&() { return id; }
    operator Hash() const { return hash; }
    operator Hash&() { return hash; }
};

} // namespace ecs

#endif // ECS_IDS_H


// end --- ids.h --- 



#include <memory>

namespace ecs {

struct EventListenerBase {
    using Ptr = std::shared_ptr<EventListenerBase>;
};

template<typename Event>
struct EventListener : public EventListenerBase {
    const Hash hash = get_type_hash<Event>();

    virtual void receive(ECS* ecs, const Event& event) = 0;
};

}    // namespace ecs_
#endif    // ECS_ECS_EVENT_H_


// end --- event.h --- 



// begin --- entity_subset.h --- 

//
// Created by Finn Eggers on 10.06.24.
//

#ifndef ECS_ENTITY_SUBSET_H
#define ECS_ENTITY_SUBSET_H

// begin --- entity_iterator.h --- 

#ifndef ECS_ECS_ITERATOR_H_
#define ECS_ECS_ITERATOR_H_

// begin --- entity.h --- 

#ifndef ECS_ECS_ENTITY_H_
#define ECS_ECS_ENTITY_H_

// begin --- component.h --- 

//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_COMPONENT_H_
#define ECS_ECS_COMPONENT_H_

// begin --- ecs_base.h --- 

//
// Created by Finn Eggers on 31.05.24.
//

#ifndef F3D_ECS_BASE_H
#define F3D_ECS_BASE_H

namespace ecs{

struct ECSBase {
    virtual void component_removed(Hash, EntityID) = 0;
    virtual void component_added(Hash, EntityID) = 0;

    virtual void entity_activated(EntityID) = 0;
    virtual void entity_deactivated(EntityID) = 0;

    virtual void destroy_entity(EntityID) = 0;
    virtual void destroy_system(SystemID) = 0;
    virtual void destroy_listener(EventListenerID) = 0;
};

}

#endif    // F3D_ECS_BASE_H


// end --- ecs_base.h --- 



#include <ostream>
#include <type_traits>
#include <memory>

namespace ecs {

/**
 * Any component that is added to an entity must inherit from this struct.
 */
struct ComponentBase {
    // empty constructor
    ComponentBase() = default;

    ECS*        ecs                 = nullptr;
    ComponentID component_id         = ComponentID {};
    ID          component_entity_id = INVALID_ID;

    // when the component is removed from the entity
    virtual void component_removed() {};

    // when the entity is activated or deactivated
    virtual void entity_activated() {};
    virtual void entity_deactivated() {};

    // when another component is added or removed
    virtual void other_component_added(Hash hash) {};
    virtual void other_component_removed(Hash hash) {};

    // get the hash of the component
    virtual Hash get_hash() const {
        return INVALID_HASH;
    };
};

template <typename T>
struct ComponentOf : public ComponentBase {
    // empty constructor
    ComponentOf() = default;

    // implement static hash function for components
    static Hash hash() {
        return get_type_hash<T>();
    }

    // override the non-static hash function to call the static one
    virtual Hash get_hash() const override {
        return hash();
    }
};

using ComponentPtr = std::unique_ptr<ComponentBase>;

} // namespace ecs_

#endif // ECS_ECS_COMPONENT_H_


// end --- component.h --- 



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
    EntityID entity_id;
    // Container for the entity's components.
    std::unordered_map<Hash, ComponentPtr> components{};
    // Pointer to the ECS that owns this entity.
    ECSBase* ecs;
    // Checks if its active or inactive.
    bool m_active = false;

    // Grant ECS access to private members.
    friend ECS;
    friend ComponentEntityList;

    public:

    // Copy constructor deleted but manual move constructor
    Entity(const Entity& other) = delete;

    // Move constructor
    Entity(Entity&& other) noexcept
        : entity_id(other.entity_id)
        , components(std::move(other.components))
        , ecs(other.ecs)
        , m_active(other.m_active) {
        other.entity_id.id = INVALID_ID;
        other.ecs = nullptr;
        other.m_active = false;
    }

    // Copy assignment operator
    Entity& operator=(const Entity& other) = delete;

    // Move assignment operator
    Entity& operator=(Entity&& other) noexcept {
        if (this == &other)
            return *this;

        remove_all_components();

        entity_id = other.entity_id;
        components = std::move(other.components);
        ecs = other.ecs;
        m_active = other.m_active;

        other.entity_id.id = INVALID_ID;
        other.ecs = nullptr;
        other.m_active = false;
        return *this;
    }

    Entity(ECSBase* p_ecs = nullptr)
        : entity_id{INVALID_ID}
        , ecs(p_ecs) {}

    virtual ~Entity() {
        remove_all_components();
    }

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
        if (components.find(hash) != components.end()) {
            return static_cast<T*>(components.at(hash).get());
        }
        return nullptr;
    }

    template<typename T, typename... Args>
    inline ComponentID assign(Args&&... args) {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        Hash hashing   = T::hash();

        // assign ecs to the component
        component->ecs = reinterpret_cast<ECS*>(ecs);
        // assign id
        component->component_id = ComponentID{entity_id, hashing};

        // If the component already exists, remove it first
        if (has<T>()) {
            remove_component<T>();
        }

        // Add the new component
        components[hashing] = std::move(component);
        ecs->component_added(hashing, id());

        // notify all other components that a new component was added
        for (auto& [hash, comp] : components) {
            // dont do it for itself
            if (hash == hashing)
                continue;

            // call the other_component_added function
            comp->other_component_added(hash);
            components[hashing]->other_component_added(hash);
        }

        // inform the component that it was added to an active entity
        if (m_active) {
            component->entity_activated();
        }

        // return the component id
        return components[hashing]->component_id;
    }

    template<typename T>
    inline void remove_component() {
        if (!has<T>())
            return;

        Hash hash = get_type_hash<T>();
        components[hash]->component_removed();
        components.erase(hash);
        ecs->component_removed(hash, id());
    }

    inline void remove_all_components() {
        for (auto& pair : components) {
            ecs->component_removed(pair.first, id());
        }
        for (auto& pair : components) {
            pair.second->component_removed();
        }

        components.clear();
    }

    inline void destroy() {
        ecs->destroy_entity(this->entity_id);
    }

    EntityID id() const {
        return entity_id;
    }

    bool valid() const {
        return entity_id != INVALID_ID;
    }

    bool active() const {
        return m_active;
    }

    void activate() {
        if (!m_active) {
            m_active = true;
            ecs->entity_activated(this->entity_id);
            for (auto& [hash, component] : components) {
                component->entity_activated();
            }
        }
    }

    void deactivate() {
        if (m_active) {
            m_active = false;
            ecs->entity_deactivated(this->entity_id);
            for (auto& [hash, component] : components) {
                component->entity_deactivated();
            }
        }
    }

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

}    // namespace ecs_

#endif    // ECS_ECS_ENTITY_H_


// end --- entity.h --- 



#include <iostream>
#include <vector>

namespace ecs {

template<typename... RTypes>
struct EntityIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Entity;
    using pointer           = value_type*;    // or also value_type*
    using reference         = value_type&;    // or also value_type&

    EntityIterator(std::vector<ID>::iterator id_iter,
                   std::vector<ID>::iterator id_end,
                   std::vector<Entity>* entity_packs)
        : m_id_iter(id_iter), m_id_end(id_end), m_entity_packs(entity_packs) {

        advance_to_next_valid();
    }

    reference operator*() const {
        return (*m_entity_packs)[*m_id_iter];
    }

    pointer operator->() {
        return &(*m_entity_packs)[*m_id_iter];
    }

    // Prefix increment
    EntityIterator& operator++() {
        ++m_id_iter;
        advance_to_next_valid();
        return *this;
    }

    // Postfix increment
    EntityIterator operator++(int) {
        EntityIterator temp = *this;
        ++(*this);
        return temp;
    }

    bool operator==(const EntityIterator& rhs) const {
        return m_id_iter == rhs.m_id_iter;
    }

    bool operator!=(const EntityIterator& rhs) const {
        return m_id_iter != rhs.m_id_iter;
    }

    private:
    std::vector<ID>::iterator m_id_iter;
    std::vector<ID>::iterator m_id_end;
    std::vector<Entity>* m_entity_packs;

    void advance_to_next_valid() {

        while (   m_id_iter != m_id_end
               && !(*m_entity_packs)[*m_id_iter].template has<RTypes...>()) {

            // if id is IVALID_ID, skip
            if (*m_id_iter == INVALID_ID) {
                ++m_id_iter;
                continue;
            }

            ++m_id_iter;
        }

    }
};
} // namespace ecs_

#endif // ECS_ECS_ITERATOR_H_


// end --- entity_iterator.h --- 



namespace ecs {

template<typename... RTypes>
struct EntitySubSet {
    std::vector<ID>* ids;
    std::vector<Entity>* entries;

    EntitySubSet(std::vector<ID>* ids, std::vector<Entity>* entries)
        : ids(ids), entries(entries) {
    }

    EntityIterator<RTypes...> begin() {
        return EntityIterator<RTypes...> {ids->begin(), ids->end(), entries};
    }

    EntityIterator<RTypes...> end() {
        return EntityIterator<RTypes...> {ids->end(), ids->end(), entries};
    }
};
}


#endif    // ECS_ENTITY_SUBSET_H


// end --- entity_subset.h --- 



// begin --- include.h --- 

//
// Created by Luecx on 16.06.2021.
//

#ifndef ECS_ECS_CORE_H_
#define ECS_ECS_CORE_H_

// begin --- ecs.h --- 

//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_ECS_H_
#define ECS_ECS_ECS_H_

// begin --- component_entity_list.h --- 

//
// Created by Finn Eggers on 07.06.24.
//

#ifndef ECS_COMPONENT_ENTITY_LIST_H
#define ECS_COMPONENT_ENTITY_LIST_H

// begin --- vector_compact.h --- 

//
// Created by Finn Eggers on 08.06.24.
//

#ifndef ECS_VECTOR_COMPACT_H
#define ECS_VECTOR_COMPACT_H

#include <vector>

// a normal vector more or less but when removing, we simply shift the last element to the position of the removed one
// using templates to generalise for storage type
namespace ecs {

template<typename T>
struct CompactVector {
    std::vector<T> elements;

    void push_back(const T &element) {
        elements.push_back(element);
    }
    void remove(const T &element) {
        // if the element is at the end, we can simply pop it
        if (element == elements.back()) {
            elements.pop_back();
            removed(element);
            return;
        }

        // check position of the element
        auto it = std::find(elements.begin(), elements.end(), element);

        // if it doesnt exist, exit here
        if (it == elements.end()) {
            return;
        }

        // otherwise we need to find the element and move the last element to its position
        if (it != elements.end()) {
            // get the id of the last element and the element to be removed
            auto last_id = elements.size() - 1;
            auto element_id = static_cast<ID>(std::distance(elements.begin(), it));

            removed(element_id);

            // move
            elements[element_id] = elements.back();
            elements.pop_back();

            moved(last_id, element_id);
        }
    }
    void remove_at(ID id) {
        // if the element is at the end, we can simply pop it
        if (id == elements.size() - 1) {
            elements.pop_back();
            removed(id);
            return;
        }

        // otherwise we need to find the element and move the last element to its position
        if (id < elements.size()) {
            // get the id of the last element and the element to be removed
            auto last_id = elements.size() - 1;

            removed(id);

            // move
            elements[id] = elements.back();
            elements.pop_back();

            moved(last_id, id);
        }
    }
    auto begin() const { return elements.begin(); }
    auto end() const { return elements.end(); }

    // access operators + at()
    T &operator[](ID id) { return elements[id]; }
    T &at(ID id) { return elements.at(id); }
    T &operator()(ID id) { return elements[id]; }
    const T &operator[](ID id) const { return elements[id]; }
    const T &at(ID id) const { return elements.at(id); }
    const T &operator()(ID id) const { return elements[id]; }

    ID size() const { return elements.size(); }
    void clear() { elements.clear(); }

    protected:

    virtual void moved(ID old_id, ID new_id) {};
    virtual void removed(ID id) {};
    virtual void added(ID id) {};
};

}    // namespace ecs_

#endif    // ECS_VECTOR_COMPACT_H


// end --- vector_compact.h --- 



#include <vector>

namespace ecs {

struct ComponentEntityList : CompactVector<ID> {
    // pointer to the ECS
    std::vector<Entity>* entities_ = nullptr;

    // hash of the component
    Hash comp_hash_ = Hash{INVALID_HASH};

    // set ecs_ and component hash via some function to allow empty constructions
    void set(std::vector<Entity>* entities, Hash component_hash) {
        this->entities_      = entities;
        this->comp_hash_     = component_hash;
    }

    // overloaded
    void moved(ID from, ID to) override {
        // TODO
//        (*entities_)[elements[to]].components[comp_hash_]->component_entity_id = to;
    }
    void removed(ID id) override {
        // TODO
//        (*entities_)[elements[id]].components[comp_hash_]->component_entity_id = INVALID_ID;
    }
    void added(ID id) override {
        // TODO
//        (*entities_)[elements[id]].components[comp_hash_]->component_entity_id = id;
    }

};

}    // namespace ecs_

#endif    // ECS_COMPONENT_ENTITY_LIST_H


// end --- component_entity_list.h --- 



// begin --- system.h --- 

//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_SYSTEM_H_
#define ECS_ECS_SYSTEM_H_

#include <memory>

namespace ecs {

struct System {
    using Ptr = std::shared_ptr<System>;

    friend struct ECS;

    protected:
    virtual void process(ECS* ecs, double delta) = 0;
    virtual void destroyed() {};
};

}    // namespace ecs_

#endif    // ECS_ECS_SYSTEM_H_


// end --- system.h --- 



// begin --- vector_recycling.h --- 

//
// Created by Finn Eggers on 08.06.24.
//

#ifndef ECS_VECTOR_RECYCLING_H
#define ECS_VECTOR_RECYCLING_H

#include <vector>
#include <queue>

// a vector that leaves gaps when elements are removed, and reuses those gaps for new elements
namespace ecs {

template<typename T>
struct RecyclingVector {
    std::vector<T> elements_;
    std::queue<ID> free_positions_;
    T default_value_;

    RecyclingVector() {};

    RecyclingVector(const T& default_val) : default_value_(default_val) {}

    ID push_back(const T &element) {
        if (!free_positions_.empty()) {
            // Reuse a free position
            ID id = free_positions_.front();
            free_positions_.pop();
            elements_[id] = element;
            added(id);
            return id;
        } else {
            // Append to the end
            elements_.push_back(element);
            added(elements_.size() - 1);
            return elements_.size() - 1;
        }
    }

    void remove(const T &element) {
        // Find the element
        auto it = std::find(elements_.begin(), elements_.end(), element);

        // If element is found, remove it
        if (it != elements_.end()) {
            ID id = std::distance(elements_.begin(), it);
            elements_[id] = default_value_;
            free_positions_.push(id);
            removed(id);
        }
    }

    void remove_at(ID id) {
        if (id < elements_.size()) {
            elements_[id] = default_value_;
            free_positions_.push(id);
            removed(id);
        }
    }

    auto begin() const {
        return elements_.begin();
    }

    auto end() const {
        return elements_.end();
    }

    // access operators + at()
    T &operator[](ID id) {
        return elements_[id];
    }

    T &at(ID id) {
        return elements_.at(id);
    }

    T &operator()(ID id) {
        return elements_[id];
    }

    const T &operator[](ID id) const {
        return elements_[id];
    }

    const T &at(ID id) const {
        return elements_.at(id);
    }

    const T &operator()(ID id) const {
        return elements_[id];
    }

    ID size() const {
        return elements_.size();
    }

    void clear() {
        elements_.clear();
        free_positions_ = std::queue<ID>();
    }

    protected:
    virtual void moved(ID old_id, ID new_id) {}
    virtual void removed(ID id) {}
    virtual void added(ID id) {}
};

} // namespace ecs

#endif    // ECS_VECTOR_RECYCLING_H


// end --- vector_recycling.h --- 



#include <algorithm>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <vector>

namespace ecs {

/**
 * @brief Represents the Entity Component System (ECS) manager.
 *
 * The ECS class manages entities, components, and systems, providing an interface
 * to create entities, add/remove components, and process systems.
 */
struct ECS : public ECSBase {
    std::unordered_map<Hash, ComponentEntityList>                     component_entity_lists {};
    std::vector<Entity>                                               entities {};
    CompactVector<ID>                                                 active_entities {};

    RecyclingVector<System::Ptr>                                      systems {nullptr};
    std::unordered_map<Hash, RecyclingVector<EventListenerBase::Ptr>> event_listener {};

    // Grant Entity access to private members.
    friend Entity;
    friend ComponentEntityList;

    ECS() = default;

    // Delete copy constructor and copy assignment operator
    ECS(const ECS&) = delete;
    ECS& operator=(const ECS&) = delete;
    // Delete move constructor and move assignment operator
    ECS(ECS&&) = delete;
    ECS& operator=(ECS&&) = delete;

    virtual ~ECS() {
        destroy_all_entities();
        destroy_all_systems();
    }

    public:
    EntityID spawn(bool active = false);
    void     destroy_entity(EntityID id) override;
    void     destroy_all_entities();
    void     destroy_all_systems();

    // operators to get an entity from an id
    Entity& operator[](ID id) {
        return entities[id];
    }
    Entity& at(ID id) {
        return entities.at(id);
    }
    Entity& operator()(ID id) {
        return entities[id];
    }

    private:
    // listeners to functions applied onto the entities
    void component_removed(Hash hash, EntityID id) override;
    void component_added(Hash hash, EntityID id) override;
    void entity_activated(EntityID id) override;
    void entity_deactivated(EntityID id) override;

    // functions to manage the lists
    void add_to_component_list(ID entity);
    void remove_from_component_list(ID entity);
    void add_to_component_list(ID entity, Hash hash);
    void remove_from_component_list(ID entity, Hash hash);
    void add_to_active_entities(ID entity);
    void remove_from_active_entities(ID entity);

    public:
    template<typename K, typename... R>
    inline EntitySubSet<K, R...> each() {
        auto  hash = K::hash();
        auto* ids  = &component_entity_lists[hash].elements;
        return EntitySubSet<K, R...> {ids, &entities};
    }

    template<typename K, typename... R>
    inline ID first() {
        for (ID id : component_entity_lists[K::hash()].entities) {
            if (entities[id].has<K, R...>()) {
                return id;
            }
        }
        return INVALID_ID;
    }

    template<typename Event>
    inline void emit_event(const Event& event) {
        Hash hash = get_type_hash<Event>();
        if (event_listener.find(hash) == event_listener.end())
            return;
        for (auto listener : event_listener[hash]) {
            auto l = reinterpret_cast<EventListener<Event>*>(listener.get());
            l->receive(this, event);
        }
    }

    template<typename T, typename... Args>
    SystemID create_system(Args&&... args) {
        std::shared_ptr<T> system = std::make_shared<T>(std::forward<Args>(args)...);
        ID pos = systems.push_back(system);
        return SystemID{pos};
    }
    void destroy_system(SystemID id) override {
        if (id >= systems.size())
            return;
        systems[id]->destroyed();
        systems.remove_at(id);
    }

    template<typename T, typename... Args>
    EventListenerID create_listener(Args&&... args) {
        std::shared_ptr<T> listener = std::make_shared<T>(std::forward<Args>(args)...);
        auto hash = listener->hash;

        if (event_listener.find(hash) == event_listener.end()) {
            event_listener[hash] = {nullptr};
        }
        ID pos = event_listener[hash].push_back(listener);
        return EventListenerID{pos, hash};
    }
    void destroy_listener(EventListenerID id) override {
        event_listener[id.operator Hash()].remove_at((ID)id);
    }

    void                 process(double delta);
    friend std::ostream& operator<<(std::ostream& os, const ECS& ecs1) {
        os << "All Entities: " << std::endl;
        os << "-----------------" << std::endl;
        for (const auto& entity : ecs1.entities) {
            os << "Entity ID: " << std::setw(10);
            if (entity.valid()) {
                os << entity.id() << " | Active: " << (entity.active() ? "true" : "false");
            } else {
                os << "INVALID"
                   << " | Active: -";
            }
            os << std::endl;
        }
        os << std::endl;

        os << "Active Entities: " << std::endl;
        os << "-----------------" << std::endl;
        for (const auto& id : ecs1.active_entities) {
            os << "Entity ID: " << std::setw(10);
            if (id != INVALID_ID) {
                os << id << " | Active: true";
            } else {
                os << "INVALID"
                   << " | Active: -";
            }
            os << std::endl;
        }
        os << std::endl;

        os << "Component Entity List: " << std::endl;
        os << "-----------------------" << std::endl;
        for (const auto& pair : ecs1.component_entity_lists) {
            os << "Component Hash: " << std::setw(20) << pair.first.hash_code() << std::endl;
            os << "Entities: " << std::endl;
            for (const auto& id : pair.second) {
                os << std::setw(10);
                if (id != INVALID_ID) {
                    os << id << " | Active: " << (ecs1.entities[id].active() ? "true" : "false");
                } else {
                    os << "INVALID_ID | Active: -";
                }
                os << std::endl;
            }
            os << "-----------------------" << std::endl;
        }
        os << std::endl;

        return os;
    }
};

}    // namespace ecs_



inline ecs::EntityID ecs::ECS::spawn(bool active) {

    entities.emplace_back(Entity{this});
    entities.back().entity_id = EntityID{entities.size() - 1};

    if (active) {
        entities.back().activate();
    }

    return entities.back().entity_id;
}

inline void ecs::ECS::destroy_entity(ecs::EntityID id) {
    // get the entity at the given id
    auto entity = &entities[id];

    // if active, deactivate (will notify components)
    entity->deactivate();

    // destroy the components
    entity->remove_all_components();

    // set the entity's id to INVALID_ID and the pointer to nullptr
    entity->entity_id = EntityID{INVALID_ID};
}

inline void ecs::ECS::destroy_all_entities() {
    for (Entity& entity : entities) {
        destroy_entity(entity.entity_id);
    }
    entities.clear();
}

inline void ecs::ECS::destroy_all_systems() {
    for (auto sys : systems) {
        sys->destroyed();
    }
    systems.clear();
}

inline void ecs::ECS::component_removed(ecs::Hash hash, EntityID id) {
    if (entities[id].active()) {
        remove_from_component_list(id, hash);
    }
}
inline void ecs::ECS::component_added(ecs::Hash hash, EntityID id) {
    if (entities[id].active()) {
        add_to_component_list(id, hash);
    }
}
inline void ecs::ECS::entity_activated(ecs::EntityID entity_id) {
    if (entity_id == INVALID_ID || entity_id >= entities.size())
        return;
    if (!entities[entity_id].valid())
        return;
    if (!entities[entity_id].active())
        return;

    add_to_active_entities(entity_id);
    add_to_component_list(entity_id);
}

inline void ecs::ECS::entity_deactivated(ecs::EntityID entity_id) {
    if (entity_id == INVALID_ID || entity_id >= entities.size())
        return;
    if (!entities[entity_id].valid())
        return;
    if (entities[entity_id].active())
        return;
    // remove from active entities
    remove_from_active_entities(entity_id);
    remove_from_component_list(entity_id);
}
inline void ecs::ECS::add_to_component_list(ID id) {
    auto entity = &entities[id];
    for (auto& [hash, component] : entity->components) {
        add_to_component_list(id, hash);
    }
}
inline void ecs::ECS::remove_from_component_list(ID id) {
    auto entity = &entities[id];
    for (auto& [hash, irrelevant] : entity->components) {
        remove_from_component_list(id, hash);
    }
}
inline void ecs::ECS::add_to_component_list(ID id, ecs::Hash hash) {
    // if the component list does not exist, create it
    if (component_entity_lists.find(hash) == component_entity_lists.end()) {
        component_entity_lists[hash] = ComponentEntityList{};
        component_entity_lists[hash].set(&entities, hash);
    }
    component_entity_lists.at(hash).push_back(id);
}
inline void ecs::ECS::remove_from_component_list(ID id, ecs::Hash hash) {
    auto entity = &entities[id];
    component_entity_lists.at(hash).remove_at(entity->components[hash]->component_entity_id);
}
inline void ecs::ECS::add_to_active_entities(ID id) {
    active_entities.push_back(id);
}
inline void ecs::ECS::remove_from_active_entities(ID id) {
    active_entities.remove(id);
}

inline void ecs::ECS::process(double delta) {
    for (auto sys : systems) {
        sys->process(this, delta);
    }
}




#endif    // ECS_ECS_ECS_H_


// end --- ecs.h --- 



#endif    // ECS_ECS_CORE_H_


// end --- include.h --- 

