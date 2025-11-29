/**
 * @file orthographic_camera.cpp
 * @brief Implements the OrthographicCamera component helpers.
 */

#include "orthographic_camera.h"

#include <algorithm>

OrthographicCamera::OrthographicCamera()
    : left_(-1.0f)
    , right_(1.0f)
    , bottom_(-1.0f)
    , top_(1.0f)
    , near_plane_(0.1f)
    , far_plane_(100.0f) {}

void OrthographicCamera::set_bounds(float left, float right, float bottom, float top) {
    left_ = left;
    right_ = right;
    bottom_ = bottom;
    top_ = top;
}

void OrthographicCamera::set_clip_planes(float near_plane, float far_plane) {
    near_plane_ = near_plane;
    far_plane_ = std::max(near_plane_ + 0.0001f, far_plane);
}

void OrthographicCamera::set_orthographic(float left, float right, float bottom, float top, float near_plane, float far_plane) {
    set_bounds(left, right, bottom, top);
    set_clip_planes(near_plane, far_plane);
}

Mat4f OrthographicCamera::projection_matrix() const {
    return Mat4f::eye().view_orthogonal(left_, right_, bottom_, top_, near_plane_, far_plane_);
}

