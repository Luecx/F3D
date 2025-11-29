/**
* @file entity_subset.h
 * @brief Range wrapper for iterating over entity IDs.
 */

#pragma once

#include <vector>
#include "types.h"
#include "entity_iterator.h"

namespace ecs {

template<typename... RTypes>
struct EntitySubSet {
    std::vector<ID>* ids;
    std::vector<Entity>* entries;

    EntitySubSet(std::vector<ID>* ids_, std::vector<Entity>* entries_)
        : ids(ids_), entries(entries_) {}

    EntityIterator<RTypes...> begin() {
        return EntityIterator<RTypes...>(ids->begin(), ids->end(), entries);
    }

    EntityIterator<RTypes...> end() {
        return EntityIterator<RTypes...>(ids->end(), ids->end(), entries);
    }
};

} // namespace ecs
