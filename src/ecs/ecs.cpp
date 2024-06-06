//
// Created by Finn Eggers on 31.05.24.
//
#include "ecs.h"

ecs::EntityPtr ecs::ECS::spawn(bool active) {
    EntityPtr entity  = std::make_shared<Entity>(this);
    entity->entity_id = entities.size();
    entities.push_back(entity);

    if (active) {
        entity->activate();
    }

    return entity;
}
void ecs::ECS::shrink() {
    // Remove invalid entities from the main entity list
    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const EntityPtr& o) { return o == nullptr || o->entity_id == INVALID_ID; }), entities.end());

    // Remove invalid entities from the active entity list
    active_entities.erase(std::remove_if(active_entities.begin(), active_entities.end(), [](const EntityPtr& o) { return o == nullptr || o->entity_id == INVALID_ID; }),
                          active_entities.end());

    // Remove invalid entities from the component entity lists
    for (auto& [hash, entityList] : component_entity_list) {
        entityList.erase(std::remove_if(entityList.begin(), entityList.end(), [](const EntityPtr& o) { return o == nullptr || o->entity_id == INVALID_ID; }), entityList.end());
    }

    // Renumber the remaining entities
    for (size_t i = 0; i < entities.size(); ++i) {
        if (entities[i] != nullptr) {
            entities[i]->entity_id = i;
        }
    }
}

void ecs::ECS::destroy() {
    destroy_all();
    for (System* sys : systems) {
        sys->destroy();
    }
    systems.clear();
}

void ecs::ECS::component_removed(ecs::Hash hash, ecs::Entity* entity) {
    if (entity->active()) {
        remove_from_component_list(entities[entity->entity_id], hash);
    }
}
void ecs::ECS::component_added(ecs::Hash hash, ecs::Entity* entity) {
    if (entity->active()) {
        add_to_component_list(entities[entity->entity_id], hash);
    }
}
void ecs::ECS::entity_activated(ecs::ID entity_id) {
    if (entity_id == INVALID_ID || entity_id >= entities.size())
        return;
    if (entities[entity_id] == nullptr)
        return;
    if (!entities[entity_id]->active())
        return;

    add_to_active_entities(entities[entity_id]);
    add_to_component_list(entities[entity_id]);
}

void ecs::ECS::entity_deactivated(ecs::ID entity_id) {
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
void ecs::ECS::add_to_component_list(ecs::EntityPtr entity) {
    for (auto& [hash, component] : entity->components) {
        add_to_component_list(entity, hash);
    }
}
void ecs::ECS::remove_from_component_list(ecs::EntityPtr entity) {
    for (auto& [hash, irrelevant] : entity->components) {
        remove_from_component_list(entity, hash);
    }
}
void ecs::ECS::add_to_component_list(ecs::EntityPtr entity, ecs::Hash hash) {
    component_entity_list[hash].push_back(entity);
}
void ecs::ECS::remove_from_component_list(ecs::EntityPtr entity, ecs::Hash hash) {
    for (auto& ent : component_entity_list[hash]) {
        if (ent && ent->entity_id == entity->entity_id) {
            ent = nullptr;
        }
    }
}
void ecs::ECS::add_to_active_entities(ecs::EntityPtr entity) {
    active_entities.push_back(entity);
}
void ecs::ECS::remove_from_active_entities(ecs::EntityPtr entity) {
    for (auto& ent : active_entities) {
        if (ent && ent->entity_id == entity->entity_id) {
            ent = nullptr;
        }
    }
}
void ecs::ECS::destroy(ecs::ID id) {
    if (id == INVALID_ID || id >= entities.size())
        return;
    if (entities[id] == nullptr)
        return;

    remove_from_active_entities(entities[id]);
    remove_from_component_list(entities[id]);

    // Set the entity's id to INVALID_ID and the pointer to nullptr
    entities[id]->entity_id = INVALID_ID;
    entities[id]->m_active  = false;
    entities[id]            = nullptr;
}
void ecs::ECS::destroy(const ecs::EntityPtr& entity) {
    if (entity == nullptr)
        return;
    destroy(entity->entity_id);
}
void ecs::ECS::destroy_all() {
    for (EntityPtr& en : entities) {
        destroy(en);
    }
    entities.clear();
}
void ecs::ECS::add_system(ecs::System* system) {
    systems.push_back(system);
}
void ecs::ECS::process(double delta) {
    for (System* sys : systems) {
        sys->process(this, delta);
    }
}
void ecs::ECS::process(ecs::ECS* ecs, double delta) {
    for (System* sys : systems) {
        sys->process(ecs, delta);
    }
}
