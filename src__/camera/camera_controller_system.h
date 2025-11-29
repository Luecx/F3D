/**
 * @file camera_controller_system.h
 * @brief Declares the system updating camera controller components.
 */

#pragma once

#include <ecs.h>

#include "camera_controller.h"

struct GLFWwindow;

/**
 * @brief System that translates GLFW input into camera controller updates.
 */
class CameraControllerSystem : public ecs::System {
  public:
    explicit CameraControllerSystem(GLFWwindow* window);

    void process(ecs::ECS* ecs, double delta) override;

  private:
    GLFWwindow* window_;
};
