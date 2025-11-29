/**
* @file entity_iterator.h
 * @brief Iterator over entities matching a component signature.
 */

#pragma once

#include <iterator>
#include <vector>
#include "types.h"
#include "entity.h"

namespace ecs {

template<typename... RTypes>
struct EntityIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Entity;
    using pointer           = value_type*;
    using reference         = value_type&;

    EntityIterator(std::vector<ID>::iterator id_iter,
                   std::vector<ID>::iterator id_end,
                   std::vector<Entity>* entity_packs)
        : m_id_iter(id_iter)
        , m_id_end(id_end)
        , m_entity_packs(entity_packs)
    {
        advance_to_next_valid();
    }

    reference operator*() const { return (*m_entity_packs)[*m_id_iter]; }
    pointer   operator->()      { return &(*m_entity_packs)[*m_id_iter]; }

    EntityIterator& operator++() {
        ++m_id_iter;
        advance_to_next_valid();
        return *this;
    }

    EntityIterator operator++(int) {
        EntityIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const EntityIterator& rhs) const { return m_id_iter == rhs.m_id_iter; }
    bool operator!=(const EntityIterator& rhs) const { return m_id_iter != rhs.m_id_iter; }

private:
    std::vector<ID>::iterator m_id_iter;
    std::vector<ID>::iterator m_id_end;
    std::vector<Entity>* m_entity_packs;

    void advance_to_next_valid() {
        while (m_id_iter != m_id_end) {
            if (*m_id_iter == INVALID_ID) {
                ++m_id_iter;
                continue;
            }

            if ((*m_entity_packs)[*m_id_iter].template has<RTypes...>()) {
                return;
            }

            ++m_id_iter;
        }
    }

};

} // namespace ecs
