/**
 * @file ecs.h
 * @brief ECS manager class tying everything together.
 */

#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "types.h"
#include "ids.h"
#include "ecs_base.h"
#include "entity.h"
#include "component_entity_list.h"
#include "system.h"
#include "event.h"
#include "vector_compact.h"
#include "vector_recycling.h"
#include "entity_subset.h"

namespace ecs {

/**
 * @brief Main ECS manager: holds entities, systems, component lists, listeners.
 */
struct ECS : public ECSBase {

    std::unordered_map<Hash, ComponentEntityList>                     component_entity_lists{};
    std::vector<Entity>                                               entities{};
    CompactVector<ID>                                                 active_entities{};

    RecyclingVector<System::Ptr>                                      systems{nullptr};
    std::unordered_map<Hash, RecyclingVector<EventListenerBase::Ptr>> event_listener{};

    friend Entity;
    friend ComponentEntityList;

    ECS()  = default;
    ~ECS() override {
        destroy_all_entities();
        destroy_all_systems();
    }

    ECS(const ECS&)            = delete;
    ECS& operator=(const ECS&) = delete;
    ECS(ECS&&)                 = delete;
    ECS& operator=(ECS&&)      = delete;

    // ---------- Entity creation / destruction ----------

    EntityID spawn(bool active = false) {
        entities.emplace_back(Entity{this});
        entities.back().entity_id = EntityID{entities.size() - 1};

        if (active) {
            entities.back().activate();
        }
        return entities.back().entity_id;
    }

    void destroy_entity(EntityID id) override {
        auto* entity = &entities[id];

        entity->deactivate();
        entity->remove_all_components();
        entity->entity_id = EntityID{INVALID_ID};
    }

    void destroy_all_entities() {
        for (Entity& entity : entities) {
            entity.deactivate();
            entity.remove_all_components();
            entity.entity_id = EntityID{INVALID_ID};
        }
        entities.clear();
    }

    void destroy_all_systems() {
        for (auto sys : systems) {
            if (sys) sys->destroyed();
        }
        systems.clear();
    }

    // ---------- Entity access ----------

    Entity& operator[](ID id) { return entities[id]; }
    Entity& at(ID id)         { return entities.at(id); }
    Entity& operator()(ID id) { return entities[id]; }

private:
    // ---------- ECSBase callbacks ----------

    void component_removed(Hash hash, EntityID id) override {
        if (entities[id].active()) {
            remove_from_component_list(id, hash);
        }
    }

    void component_added(Hash hash, EntityID id) override {
        if (entities[id].active()) {
            add_to_component_list(id, hash);
        }
    }

    void entity_activated(EntityID entity_id) override {
        if (entity_id == INVALID_ID || entity_id >= entities.size()) return;
        if (!entities[entity_id].valid()) return;
        if (!entities[entity_id].active()) return;

        add_to_active_entities(entity_id);
        add_to_component_list(entity_id);
    }

    void entity_deactivated(EntityID entity_id) override {
        if (entity_id == INVALID_ID || entity_id >= entities.size()) return;
        if (!entities[entity_id].valid()) return;
        if (entities[entity_id].active()) return;

        remove_from_active_entities(entity_id);
        remove_from_component_list(entity_id);
    }

    // ---------- list management ----------

    void add_to_component_list(ID id) {
        auto* entity = &entities[id];
        for (auto& [hash, component] : entity->components) {
            (void)component;
            add_to_component_list(id, hash);
        }
    }

    void remove_from_component_list(ID id) {
        auto* entity = &entities[id];
        for (auto& [hash, irrelevant] : entity->components) {
            (void)irrelevant;
            remove_from_component_list(id, hash);
        }
    }

    void add_to_component_list(ID id, Hash hash) {
        if (component_entity_lists.find(hash) == component_entity_lists.end()) {
            component_entity_lists[hash] = ComponentEntityList{};
            component_entity_lists[hash].set(&entities, hash);
        }
        component_entity_lists.at(hash).push_back(id);
    }

    void remove_from_component_list(ID id, Hash hash) {
        auto* entity = &entities[id];
        component_entity_lists.at(hash)
            .remove_at(entity->components[hash]->component_entity_id);
    }

    void add_to_active_entities(ID id) {
        active_entities.push_back(id);
    }

    void remove_from_active_entities(ID id) {
        active_entities.remove(id);
    }

public:
    // ---------- high-level iteration ----------

    template<typename K, typename... R>
    EntitySubSet<K, R...> each() {
        auto  hash = K::hash();
        auto* ids  = &component_entity_lists[hash].elements;
        return EntitySubSet<K, R...>{ids, &entities};
    }

    template<typename K, typename... R>
    ID first() {
        for (ID id : component_entity_lists[K::hash()].elements) {
            if (entities[id].has<K, R...>()) {
                return id;
            }
        }
        return INVALID_ID;
    }

    // ---------- events ----------

    template<typename Event>
    void emit_event(const Event& event) {
        Hash hash = get_type_hash<Event>();
        auto it = event_listener.find(hash);
        if (it == event_listener.end()) return;

        for (auto listener : it->second) {
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
        if (id >= systems.size()) return;
        systems[id]->destroyed();
        systems.remove_at(id);
    }

    template<typename T, typename... Args>
    EventListenerID create_listener(Args&&... args) {
        std::shared_ptr<T> listener = std::make_shared<T>(std::forward<Args>(args)...);
        auto hash = listener->hash;

        if (event_listener.find(hash) == event_listener.end()) {
            event_listener[hash] = RecyclingVector<EventListenerBase::Ptr>{nullptr};
        }
        ID pos = event_listener[hash].push_back(listener);
        return EventListenerID{pos, hash};
    }

    void destroy_listener(EventListenerID id) override {
        event_listener[id.operator Hash()].remove_at(static_cast<ID>(id));
    }

    // ---------- system processing ----------

    void process(double delta) {
        for (auto sys : systems) {
            if (sys) sys->process(this, delta);
        }
    }

    // ---------- debug print ----------

    friend std::ostream& operator<<(std::ostream& os, const ECS& ecs1) {
        os << "All Entities:\n-----------------\n";
        for (const auto& entity : ecs1.entities) {
            os << "Entity ID: " << std::setw(10);
            if (entity.valid()) {
                os << entity.id().id << " | Active: " << (entity.active() ? "true" : "false");
            } else {
                os << "INVALID | Active: -";
            }
            os << '\n';
        }
        os << '\n';

        os << "Active Entities:\n-----------------\n";
        for (const auto& id : ecs1.active_entities.elements) {
            os << "Entity ID: " << std::setw(10);
            if (id != INVALID_ID) {
                os << id << " | Active: true";
            } else {
                os << "INVALID | Active: -";
            }
            os << '\n';
        }
        os << '\n';

        os << "Component Entity List:\n-----------------------\n";
        for (const auto& pair : ecs1.component_entity_lists) {
            os << "Component Hash: " << std::setw(20) << pair.first.hash_code() << '\n';
            os << "Entities:\n";
            for (const auto& id : pair.second.elements) {
                os << std::setw(10);
                if (id != INVALID_ID) {
                    os << id << " | Active: "
                       << (ecs1.entities[id].active() ? "true" : "false");
                } else {
                    os << "INVALID_ID | Active: -";
                }
                os << '\n';
            }
            os << "-----------------------\n";
        }
        os << '\n';
        return os;
    }
};

} // namespace ecs
