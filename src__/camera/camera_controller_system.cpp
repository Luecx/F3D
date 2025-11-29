/**
 * @file camera_controller_system.cpp
 * @brief Implements the system that applies user input to cameras.
 */

#include "camera_controller_system.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

#include "../logging/logging.h"

namespace {
constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
constexpr float kRadToDeg = 180.0f / 3.14159265358979323846f;

Vec3f direction_from_angles(float pitch_deg, float yaw_deg) {
    float pitch = pitch_deg * kDegToRad;
    float yaw = yaw_deg * kDegToRad;

    float cos_pitch = std::cos(pitch);
    Vec3f forward{std::sin(yaw) * cos_pitch,
                  std::sin(pitch),
                  -std::cos(yaw) * cos_pitch};
    return forward.normalised();
}
} // namespace

CameraControllerSystem::CameraControllerSystem(GLFWwindow* window)
    : window_(window) {}

void CameraControllerSystem::process(ecs::ECS* ecs, double delta) {
    if (!window_ || !ecs) {
        return;
    }

    double mouse_x = 0.0;
    double mouse_y = 0.0;
    glfwGetCursorPos(window_, &mouse_x, &mouse_y);

    float delta_seconds = static_cast<float>(delta);

    for (auto& entity : ecs->each<CameraController>()) {
        auto* controller = entity.get<CameraController>();
        auto* transform = entity.get<Transformation>();

        if (!controller || !transform || !controller->active()) {
            continue;
        }

        bool toggle_key = glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS;
        if (toggle_key && !controller->mode_toggle_pressed) {
            CameraController::Mode next = controller->mode() == CameraController::Mode::Fly
                                              ? CameraController::Mode::Orbit
                                              : CameraController::Mode::Fly;
            controller->set_mode(next);
            controller->mode_toggle_pressed = true;
            controller->orbit_distance = (transform->local_position() - controller->orbit_target).length();
            controller->orbit_distance = std::max(controller->orbit_distance, 0.1f);

            Vec3f to_target = controller->orbit_target - transform->local_position();
            if (to_target.length() > 1e-4f) {
                Vec3f dir = to_target.normalised();
                controller->orbit_pitch = std::asin(std::clamp(dir[1], -1.0f, 1.0f)) * kRadToDeg;
                controller->orbit_yaw = std::atan2(dir[0], -dir[2]) * kRadToDeg;
            } else {
                Vec3f current_rot = transform->local_rotation();
                controller->orbit_pitch = current_rot[0];
                controller->orbit_yaw = current_rot[1];
            }

            if (next == CameraController::Mode::Orbit) {
                transform->set_look_at(transform->local_position(), controller->orbit_target);
            }

            std::string mode_name = next == CameraController::Mode::Fly ? "Fly" : "Orbit";
            logging::log(0, logging::INFO, "CameraController: switched to " + mode_name + " mode");
        } else if (!toggle_key) {
            controller->mode_toggle_pressed = false;
        }

        bool moved = false;
        bool rotated = false;

        Vec3f rotation = transform->local_rotation();

        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (controller->first_mouse) {
                controller->last_mouse_x = mouse_x;
                controller->last_mouse_y = mouse_y;
                controller->first_mouse = false;
            }

            double offset_x = mouse_x - controller->last_mouse_x;
            double offset_y = mouse_y - controller->last_mouse_y;
            controller->last_mouse_x = mouse_x;
            controller->last_mouse_y = mouse_y;

            if (controller->mode() == CameraController::Mode::Orbit) {
                controller->orbit_yaw -= static_cast<float>(offset_x) * controller->look_sensitivity;
                controller->orbit_pitch -= static_cast<float>(offset_y) * controller->look_sensitivity;
                controller->orbit_pitch = std::clamp(controller->orbit_pitch, -89.0f, 89.0f);
            } else {
                rotation[1] -= static_cast<float>(offset_x) * controller->look_sensitivity;
                rotation[0] -= static_cast<float>(offset_y) * controller->look_sensitivity;
                rotation[0] = std::clamp(rotation[0], -89.0f, 89.0f);
                transform->set_rotation(rotation);
            }

            rotated = true;
        } else {
            controller->first_mouse = true;
        }

        Vec3f position = transform->local_position();
        if (controller->mode() == CameraController::Mode::Fly) {
            Vec3f forward = -(transform->global_zaxis().normalised());
            Vec3f right = transform->global_xaxis().normalised();
            Vec3f up = transform->global_yaxis().normalised();
            float velocity = controller->move_speed * delta_seconds;

            if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
                position += forward * velocity;
                moved = true;
            }
            if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
                position -= forward * velocity;
                moved = true;
            }
            if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
                position -= right * velocity;
                moved = true;
            }
            if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
                position += right * velocity;
                moved = true;
            }
            if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) {
                position -= up * velocity;
                moved = true;
            }
            if (glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS) {
                position += up * velocity;
                moved = true;
            }

            if (moved) {
                transform->set_position(position);
            }
        } else {
            float zoom_velocity = controller->orbit_zoom_speed * delta_seconds;
            if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
                controller->orbit_distance = std::max(0.2f, controller->orbit_distance - zoom_velocity);
            }
            if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
                controller->orbit_distance += zoom_velocity;
            }
            Vec3f orbit_dir = direction_from_angles(controller->orbit_pitch, controller->orbit_yaw);
            if (orbit_dir.length() < 1e-4f) {
                orbit_dir = Vec3f{0.0f, 0.0f, -1.0f};
            }
            Vec3f new_position = controller->orbit_target - orbit_dir.normalised() * controller->orbit_distance;
            position = new_position;
            transform->set_look_at(position, controller->orbit_target);
            rotation = transform->local_rotation();
            moved = true;
        }

        controller->log_timer += delta_seconds;
        if ((moved || rotated) && controller->log_timer >= controller->log_interval) {
            controller->log_timer = 0.0f;
            std::ostringstream ss;
            ss << "Camera position: ("
               << position[0] << ", " << position[1] << ", " << position[2]
               << ") rotation: (" << rotation[0] << ", " << rotation[1] << ", " << rotation[2] << ")";
            logging::log(0, logging::INFO, ss.str());
        }
    }
}

