/**
 * @file vector_compact.h
 * @brief CompactVector: dense vector with erase-by-swap-back.
 */

#pragma once

#include <algorithm>
#include <vector>
#include "types.h"

namespace ecs {

/**
 * @brief Vector that keeps elements compact by swapping with the last on erase.
 */
template<typename T>
struct CompactVector {
    std::vector<T> elements;

    void push_back(const T& element) {
        elements.push_back(element);
        added(elements.size() - 1);
    }

    void remove(const T& element) {
        if (elements.empty()) return;

        if (element == elements.back()) {
            elements.pop_back();
            removed(elements.size());
            return;
        }

        auto it = std::find(elements.begin(), elements.end(), element);
        if (it == elements.end()) return;

        auto last_id    = elements.size() - 1;
        auto element_id = static_cast<ID>(std::distance(elements.begin(), it));

        removed(element_id);

        elements[element_id] = elements.back();
        elements.pop_back();

        moved(last_id, element_id);
    }

    void remove_at(ID id) {
        if (id >= elements.size()) return;

        if (id == elements.size() - 1) {
            elements.pop_back();
            removed(id);
            return;
        }

        auto last_id = elements.size() - 1;

        removed(id);

        elements[id] = elements.back();
        elements.pop_back();

        moved(last_id, id);
    }

    auto begin() const { return elements.begin(); }
    auto end()   const { return elements.end(); }

    T& operator[](ID id)       { return elements[id]; }
    T& at(ID id)               { return elements.at(id); }
    T& operator()(ID id)       { return elements[id]; }
    const T& operator[](ID id) const { return elements[id]; }
    const T& at(ID id)   const { return elements.at(id); }
    const T& operator()(ID id) const { return elements[id]; }

    ID  size()  const { return elements.size(); }
    void clear()      { elements.clear(); }

protected:
    virtual void moved(ID /*old_id*/, ID /*new_id*/) {}
    virtual void removed(ID /*id*/) {}
    virtual void added(ID /*id*/) {}
};

} // namespace ecs
