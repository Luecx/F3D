/**
 * @file camera_controller.cpp
 * @brief Implements the CameraController component helpers.
 */

#include "camera_controller.h"

CameraController::CameraController()
    : move_speed(5.0f), look_sensitivity(0.15f), log_interval(0.5f), orbit_zoom_speed(5.0f), orbit_distance(5.0f),
      orbit_target{0.0f, 0.0f, 0.0f}, orbit_yaw(0.0f), orbit_pitch(0.0f), first_mouse(true), last_mouse_x(0.0),
      last_mouse_y(0.0), log_timer(0.0f), mode_toggle_pressed(false), active_(true), mode_(Mode::Fly) {}

void CameraController::set_active(bool active) { active_ = active; }

bool CameraController::active() const { return active_; }

void CameraController::set_mode(Mode mode) { mode_ = mode; }

CameraController::Mode CameraController::mode() const { return mode_; }
