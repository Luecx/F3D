//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_ECS_H_
#define ECS_ECS_ECS_H_

#include "component.h"
#include "entity.h"
#include "event.h"
#include "hash.h"
#include "iterator.h"
#include "system.h"
#include "types.h"

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
struct ECS : public System{
    std::unordered_map<Hash, std::vector<EntityPtr>>          component_entity_list {};    ///< Map of component hashes to active entities with that component.
    std::vector<EntityPtr>                                    entities {};                 ///< List of all entities.
    std::vector<EntityPtr>                                    active_entities {};          ///< List of all active entities.
    std::vector<System*>                                      systems {};                  ///< List of all systems.
    std::unordered_map<Hash, std::vector<EventBaseListener*>> event_listener {};           ///< Map of event hashes to event listeners.

    // Grant Entity access to private members.
    friend Entity;

    /**
     * @brief Destructor for the ECS. Destroys all entities and systems.
     */
    virtual ~ECS() {
        destroy_all();
    }

    /**
     * @brief Spawns a new entity and adds it to the ECS.
     *
     * @return Pointer to the newly created entity.
     */
    EntityPtr spawn(bool active = false) {
        EntityPtr entity = std::make_shared<Entity>(this);
        entity->entity_id = entities.size();
        entities.push_back(entity);

        if (active) {
            entity->activate();
        }

        return entity;
    }


    /**
     * @brief Shrinks the indices by removing invalid entities and renumbering the remaining entities.
     */
    void shrink() {
        // Remove invalid entities from the main entity list
        entities.erase(std::remove_if(entities.begin(), entities.end(), [](const EntityPtr& o) {
                           return o == nullptr || o->entity_id == INVALID_ID;
                       }), entities.end());

        // Remove invalid entities from the active entity list
        active_entities.erase(std::remove_if(active_entities.begin(), active_entities.end(), [](const EntityPtr& o) {
                                  return o == nullptr || o->entity_id == INVALID_ID;
                              }), active_entities.end());

        // Remove invalid entities from the component entity lists
        for (auto& [hash, entityList] : component_entity_list) {
            entityList.erase(std::remove_if(entityList.begin(), entityList.end(), [](const EntityPtr& o) {
                                 return o == nullptr || o->entity_id == INVALID_ID;
                             }), entityList.end());
        }

        // Renumber the remaining entities
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i] != nullptr) {
                entities[i]->entity_id = i;
            }
        }
    }


    /**
     * @brief Destroys all entities and clears all systems.
     */
    void destroy() {
        destroy_all();
        for (System* sys : systems) {
            sys->destroy();
        }
        systems.clear();
    }

    private:
    /**
     * @brief Handles the removal of a component from an entity.
     *
     * @param hash The hash of the component type.
     * @param entity The entity from which the component is removed.
     */
    void component_removed(Hash hash, Entity* entity) {
        if (entity->active()) {
            remove_from_component_list(entities[entity->entity_id], hash);
        }
    }

    /**
     * @brief Template function to handle the removal of a specific component type from an entity.
     *
     * @tparam C The type of the component.
     * @param entity The entity from which the component is removed.
     */
    template<typename C>
    void component_removed(Entity* entity) {
        Hash hash = get_type_hash<C>();
        component_removed(hash, entity);
    }

    /**
     * @brief Template function to handle the addition of a specific component type to an entity.
     *
     * @tparam C The type of the component.
     * @param entity The entity to which the component is added.
     */
    template<typename C>
    void component_added(Entity* entity) {
        if (entity->active()) {
            Hash hash = get_type_hash<C>();

            add_to_component_list(entities[entity->entity_id], hash);
        }
    }

    /**
     * @brief Adds an entity to the ECS.
     * @param entity_id
     */
    void entity_activated(ID entity_id) {
        if (entity_id == INVALID_ID || entity_id >= entities.size())
            return;
        if (entities[entity_id] == nullptr)
            return;
        if (!entities[entity_id]->active())
            return;

        add_to_active_entities(entities[entity_id]);
        add_to_component_list(entities[entity_id]);
    }

    /**
     * @brief Removes an entity from the ECS.
     * @param entity_id
     */
    void entity_deactivated(ID entity_id) {
        if (entity_id == INVALID_ID || entity_id >= entities.size())
            return;
        if (entities[entity_id] == nullptr)
            return;
        if (entities[entity_id]->active())
            return;
        // set nullptr in active entities at given index
        for (auto& entity : active_entities) {
            if (entity && entity->entity_id == entity_id) {
                entity = nullptr;
            }
        }

        remove_from_component_list(entities[entity_id]);

    }

    // functions to manage the lists
    void add_to_component_list(EntityPtr entity) {
        for (auto& [hash, component] : entity->components) {
            add_to_component_list(entity, hash);
        }
    }
    // functions to remove from component lists
    void remove_from_component_list(EntityPtr entity) {
        for (auto& [hash, irrelevant] : entity->components) {
            remove_from_component_list(entity, hash);
        }
    }
    // function to add to a single component list
    void add_to_component_list(EntityPtr entity, Hash hash) {
        component_entity_list[hash].push_back(entity);
    }
    // function to remove from a single component list
    void remove_from_component_list(EntityPtr entity, Hash hash) {
        for (auto& entity : component_entity_list[hash]) {
            if (entity && entity->entity_id == entity->entity_id) {
                entity = nullptr;
            }
        }
    }
    // function to add to active entities
    void add_to_active_entities(EntityPtr entity) {
        active_entities.push_back(entity);
    }
    // function to remove from active entities
    void remove_from_active_entities(EntityPtr entity) {
        for (auto& entity : active_entities) {
            if (entity && entity->entity_id == entity->entity_id) {
                entity = nullptr;
            }
        }
    }

    public:
    /**
     * @brief Destroys an entity by its ID.
     *
     * @param id The ID of the entity to destroy.
     */
    void destroy(ID id) {
        if (id == INVALID_ID || id >= entities.size())
            return;
        if (entities[id] == nullptr)
            return;

        remove_from_active_entities(entities[id]);
        remove_from_component_list(entities[id]);

        // Set the entity's id to INVALID_ID and the pointer to nullptr
        entities[id]->entity_id = INVALID_ID;
        entities[id]->m_active = false;
        entities[id] = nullptr;
    }


    /**
     * @brief Destroys the specified entity.
     *
     * @param entity The entity to destroy.
     */
    void destroy(const EntityPtr& entity) {
        if (entity == nullptr)
            return;
        destroy(entity->entity_id);
    }

    /**
     * @brief Destroys all entities.
     */
    void destroy_all() {
        for (EntityPtr& en : entities) {
            destroy(en);
        }
        entities.clear();
    }

    /**
     * @brief Returns a subset of entities that have the specified components.
     *
     * @tparam K The first component type.
     * @tparam R The rest of the component types.
     * @return EntitySubSet<K, R...> The subset of entities with the specified components.
     */
    template<typename K, typename... R>
    EntitySubSet<K, R...> each() {
        return EntitySubSet<K, R...> {&component_entity_list[get_type_hash<K>()]};
    }

    /**
     * @brief Returns the first entity that has the specified components.
     *
     * @tparam K The first component type.
     * @tparam R The rest of the component types.
     * @return EntityPtr Pointer to the first entity with the specified components, or nullptr if none found.
     */
    template<typename K, typename... R>
    EntityPtr first() {
        for (EntityPtr entity : component_entity_list[get_type_hash<K>()]) {
            if (entity->has<K, R...>()) {
                return entity;
            }
        }
        return nullptr;
    }

    /**
     * @brief Adds an event listener for the specified event type.
     *
     * @tparam Event The event type.
     * @param listener Pointer to the event listener.
     */
    template<typename Event>
    void add_event_listener(EventListener<Event>* listener) {
        Hash hash = get_type_hash<Event>();
        if (event_listener.find(hash) == event_listener.end()) {
            event_listener[hash] = {};
        }
        event_listener[hash].push_back(listener);
    }

    /**
     * @brief Emits an event to all listeners of the specified event type.
     *
     * @tparam Event The event type.
     * @param event The event to emit.
     */
    template<typename Event>
    void emit_event(const Event& event) {
        Hash hash = get_type_hash<Event>();
        if (event_listener.find(hash) == event_listener.end())
            return;
        for (EventBaseListener* listener : event_listener[hash]) {
            auto l = reinterpret_cast<EventListener<Event>*>(listener);
            l->receive(event);
        }
    }

    /**
     * @brief Adds a system to the ECS.
     *
     * @param system Pointer to the system to add.
     */
    void add_system(System* system) {
        systems.push_back(system);
    }

    /**
     * @brief Processes all systems with the given time delta.
     *
     * @param delta The time delta for the update.
     */
    void process(double delta) {
        for (System* sys : systems) {
            sys->process(this, delta);
        }
    }

    /**
     * @brief Overrides the System process method to process all systems.
     *
     * @param ecs Pointer to the ECS.
     * @param delta The time delta for the update.
     */
    void process(ECS* ecs, double delta) override {
        for (System* sys : systems) {
            sys->process(ecs, delta);
        }
    }

    /**
     * @brief Overloaded stream insertion operator for the ECS.
     *
     * @param os The output stream.
     * @param ecs1 The ECS to output.
     * @return The output stream with the ECS information.
     */
    /**
 * @brief Overloaded stream insertion operator for the ECS.
 *
 * @param os The output stream.
 * @param ecs1 The ECS to output.
 * @return The output stream with the ECS information.
     */
    friend std::ostream& operator<<(std::ostream& os, const ECS& ecs1) {
        os << "All Entities: " << std::endl;
        os << "-----------------" << std::endl;
        for (const auto& entity : ecs1.entities) {
            if (entity) {
                os << "Entity ID: " << std::setw(10) << entity->entity_id
                   << " | Active: " << (entity->active() ? "true" : "false") << std::endl;
            } else {
                os << "Entity ID: " << std::setw(10) << "null"
                   << " | Active: " << "-" << std::endl;
            }
        }
        os << std::endl;

        os << "Active Entities: " << std::endl;
        os << "-----------------" << std::endl;
        for (const auto& entity : ecs1.active_entities) {
            if (entity) {
                os << "Entity ID: " << std::setw(10) << entity->entity_id
                   << " | Active: " << "true" << std::endl;
            } else {
                os << "Entity ID: " << std::setw(10) << "null"
                   << " | Active: " << "-" << std::endl;
            }
        }
        os << std::endl;

        os << "Component Entity List: " << std::endl;
        os << "-----------------------" << std::endl;
        for (const auto& pair : ecs1.component_entity_list) {
            os << "Component Hash: " << std::setw(20) << pair.first.hash_code() << std::endl;
            os << "Entities: ";
            for (const auto& entity : pair.second) {
                if (entity) {
                    os << std::setw(10) << entity->entity_id << " (Active: "
                       << (entity->active() ? "true" : "false") << ") ";
                } else {
                    os << std::setw(10) << "null (Active: -) ";
                }
            }
            os << std::endl << "-----------------------" << std::endl;
        }
        os << std::endl;

        return os;
    }

};

}    // namespace ecs

#endif    // ECS_ECS_ECS_H_
