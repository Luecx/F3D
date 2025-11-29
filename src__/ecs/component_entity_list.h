/**
* @file component_entity_list.h
 * @brief Dense list of entities owning a specific component type.
 */

#pragma once

#include <vector>
#include "types.h"
#include "vector_compact.h"
#include "entity.h"

namespace ecs {

/**
 * @brief ComponentEntityList keeps entity IDs for one component type and
 *        maintains ComponentBase::component_entity_id.
 */
struct ComponentEntityList : CompactVector<ID> {
    std::vector<Entity>* entities_ = nullptr;
    Hash comp_hash_ = Hash{INVALID_HASH};

    void set(std::vector<Entity>* entities, Hash component_hash) {
        entities_  = entities;
        comp_hash_ = component_hash;
    }

protected:
    void moved(ID /*from*/, ID to) override {
        if (!entities_ || to >= elements.size()) return;
        ID entity_id = elements[to];
        if (entity_id == INVALID_ID) return;

        auto& entity = (*entities_)[entity_id];
        auto it = entity.components.find(comp_hash_);
        if (it != entity.components.end()) {
            it->second->component_entity_id = to;
        }
    }

    void removed(ID id) override {
        if (!entities_ || id >= elements.size()) return;
        ID entity_id = elements[id];
        if (entity_id == INVALID_ID) return;

        auto& entity = (*entities_)[entity_id];
        auto it = entity.components.find(comp_hash_);
        if (it != entity.components.end()) {
            it->second->component_entity_id = INVALID_ID;
        }
    }

    void added(ID id) override {
        if (!entities_ || id >= elements.size()) return;
        ID entity_id = elements[id];
        if (entity_id == INVALID_ID) return;

        auto& entity = (*entities_)[entity_id];
        auto it = entity.components.find(comp_hash_);
        if (it != entity.components.end()) {
            it->second->component_entity_id = id;
        }
    }
};

} // namespace ecs
