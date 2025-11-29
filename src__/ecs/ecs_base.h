/**
* @file ecs_base.h
 * @brief Internal ECSBase interface implemented by ECS.
 */

#pragma once

#include "types.h"
#include "ids.h"

namespace ecs {

/**
 * @brief Internal ECS interface for callbacks from entities/components.
 *
 * Users never touch this directly.
 */
struct ECSBase {
  virtual void component_removed(Hash, EntityID) = 0;
  virtual void component_added(Hash,  EntityID)  = 0;

  virtual void entity_activated(  EntityID) = 0;
  virtual void entity_deactivated(EntityID) = 0;

  virtual void destroy_entity( EntityID)      = 0;
  virtual void destroy_system( SystemID)      = 0;
  virtual void destroy_listener(EventListenerID) = 0;

  virtual ~ECSBase() = default;
};

} // namespace ecs
