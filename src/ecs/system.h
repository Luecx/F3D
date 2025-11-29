/**
* @file system.h
 * @brief Base class for ECS systems.
 */

#pragma once

#include <memory>
#include "types.h"

namespace ecs {

struct ECS;

/**
 * @brief Base class for all ECS logic systems.
 */
struct System {
  using Ptr = std::shared_ptr<System>;

  friend struct ECS;

protected:
  virtual void process(ECS* ecs, double delta) = 0;
  virtual void destroyed() {}
};

} // namespace ecs
