/**
* @file ids.h
 * @brief Strong wrapper ID types for entities, components, systems, listeners.
 */

#pragma once

#include "types.h"

namespace ecs {

struct EntityID {
    ID id = INVALID_ID;
    operator ID() const { return id; }
    operator ID&() { return id; }
};

struct ComponentID {
    ID   id   = INVALID_ID;
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
