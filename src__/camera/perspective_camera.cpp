/**
 * @file perspective_camera.cpp
 * @brief Defines the PerspectiveCamera component helpers.
 */

#include "perspective_camera.h"

#include <algorithm>

PerspectiveCamera::PerspectiveCamera()
    : fov_deg_(60.0f)
    , near_plane_(0.1f)
    , far_plane_(200.0f)
    , viewport_width_(1280)
    , viewport_height_(720) {}

void PerspectiveCamera::set_perspective(float fov_deg, float near_plane, float far_plane) {
    fov_deg_ = fov_deg;
    near_plane_ = std::max(0.0001f, near_plane);
    far_plane_ = std::max(near_plane_ + 0.0001f, far_plane);
}

void PerspectiveCamera::set_viewport(int width, int height) {
    viewport_width_ = std::max(1, width);
    viewport_height_ = std::max(1, height);
}

Mat4f PerspectiveCamera::projection_matrix() const {
    return Mat4f::eye().view_perspective(fov_deg_, aspect_ratio(), near_plane_, far_plane_);
}

float PerspectiveCamera::aspect_ratio() const {
    if (viewport_height_ == 0) {
        return 1.0f;
    }
    return static_cast<float>(viewport_width_) / static_cast<float>(viewport_height_);
}

