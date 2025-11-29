#pragma once

/**
 * @file camera_controller.h
 * @brief Declares the CameraController component handling user input state.
 */

#include <ecs.h>

#include "../math/transformation.h"

/**
 * @brief Component storing camera movement input parameters.
 */
class CameraController : public ecs::ComponentOf<CameraController> {
  public:
    /** @brief Interaction modes supported by the controller. */
    enum class Mode { Fly, Orbit };

    CameraController();

    /** @brief Enables or disables processing of this controller. */
    void set_active(bool active);

    /** @brief Returns whether the controller reacts to input. */
    bool active() const;

    /** @brief Switches the controller mode. */
    void set_mode(Mode mode);

    /** @brief Current controller mode. */
    Mode mode() const;

    bool is_orbit_mode() const { return mode_ == Mode::Orbit; }

    float move_speed;
    float look_sensitivity;
    float log_interval;
    float orbit_zoom_speed;

    float orbit_distance;
    Vec3f orbit_target;
    float orbit_yaw;
    float orbit_pitch;

    bool first_mouse;
    double last_mouse_x;
    double last_mouse_y;
    float log_timer;
    bool mode_toggle_pressed;

  private:
    bool active_;
    Mode mode_;
};
