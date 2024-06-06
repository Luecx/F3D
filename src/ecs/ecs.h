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
#include "ecs_base.h"

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
struct ECS : public System, public ECSBase {
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
    EntityPtr spawn(bool active = false);

    /**
     * @brief Shrinks the indices by removing invalid entities and renumbering the remaining entities.
     */
    void shrink();

    /**
     * @brief Destroys all entities and clears all systems.
     */
    void destroy() override;

    private:
    /**
     * @brief Handles the removal of a component from an entity.
     *
     * @param hash The hash of the component type.
     * @param entity The entity from which the component is removed.
     */
    void component_removed(Hash hash, Entity* entity) override;

    /**
     * @brief Template function to handle the addition of a specific component type to an entity.
     * @param hash
     * @param entity
     */
    void component_added(Hash hash, Entity* entity) override;

    /**
     * @brief Adds an entity to the ECS.
     * @param entity_id
     */
    void entity_activated(ID entity_id) override;

    /**
     * @brief Removes an entity from the ECS.
     * @param entity_id
     */
    void entity_deactivated(ID entity_id) override;

    // functions to manage the lists
    void add_to_component_list(EntityPtr entity);
    // functions to remove from component lists
    void remove_from_component_list(EntityPtr entity);
    // function to add to a single component list
    void add_to_component_list(EntityPtr entity, Hash hash);
    // function to remove from a single component list
    void remove_from_component_list(EntityPtr entity, Hash hash);
    // function to add to active entities
    void add_to_active_entities(EntityPtr entity);
    // function to remove from active entities
    void remove_from_active_entities(EntityPtr entity);

    public:
    /**
     * @brief Destroys an entity by its ID.
     *
     * @param id The ID of the entity to destroy.
     */
    void destroy(ID id) override;

    /**
     * @brief Destroys the specified entity.
     *
     * @param entity The entity to destroy.
     */
    void destroy(const EntityPtr& entity);

    /**
     * @brief Destroys all entities.
     */
    void destroy_all();

    /**
     * @brief Returns a subset of entities that have the specified components.
     *
     * @tparam K The first component type.
     * @tparam R The rest of the component types.
     * @return EntitySubSet<K, R...> The subset of entities with the specified components.
     */
    template<typename K, typename... R>
    inline EntitySubSet<K, R...> each() {
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
    inline EntityPtr first() {
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
    inline void add_event_listener(EventListener<Event>* listener) {
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
    inline void emit_event(const Event& event) {
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
    void add_system(System* system);

    /**
     * @brief Processes all systems with the given time delta.
     *
     * @param delta The time delta for the update.
     */
    void process(double delta);

    /**
     * @brief Overrides the System process method to process all systems.
     *
     * @param ecs Pointer to the ECS.
     * @param delta The time delta for the update.
     */
    void process(ECS* ecs, double delta) override;

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
                os << "Entity ID: " << std::setw(10) << entity->entity_id << " | Active: " << (entity->active() ? "true" : "false") << std::endl;
            } else {
                os << "Entity ID: " << std::setw(10) << "null"
                   << " | Active: "
                   << "-" << std::endl;
            }
        }
        os << std::endl;

        os << "Active Entities: " << std::endl;
        os << "-----------------" << std::endl;
        for (const auto& entity : ecs1.active_entities) {
            if (entity) {
                os << "Entity ID: " << std::setw(10) << entity->entity_id << " | Active: "
                   << "true" << std::endl;
            } else {
                os << "Entity ID: " << std::setw(10) << "null"
                   << " | Active: "
                   << "-" << std::endl;
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
                    os << std::setw(10) << entity->entity_id << " (Active: " << (entity->active() ? "true" : "false") << ") ";
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
