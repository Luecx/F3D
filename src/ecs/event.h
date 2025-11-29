/**
* @file event.h
 * @brief ECS event base and listener interface.
 */

#pragma once

#include <memory>
#include "types.h"
#include "hash.h"
#include "ids.h"

namespace ecs {

struct ECS;

/**
 * @brief Base class for all event listener registry entries.
 */
struct EventListenerBase {
  using Ptr = std::shared_ptr<EventListenerBase>;
  virtual ~EventListenerBase() = default;
};

/**
 * @brief Listener for events of type Event.
 *
 * Derive from this and implement receive().
 */
template<typename Event>
struct EventListener : public EventListenerBase {
  const Hash hash = get_type_hash<Event>();
  virtual void receive(ECS* ecs, const Event& event) = 0;
};

} // namespace ecs
