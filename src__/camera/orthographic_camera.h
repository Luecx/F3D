/**
 * @file orthographic_camera.h
 * @brief Declares the OrthographicCamera component.
 */

#pragma once

#include "../math/mat.h"
#include "../math/transformation.h"

/**
 * @brief Orthographic projection camera component.
 */
class OrthographicCamera : public ecs::ComponentOf<OrthographicCamera> {
  public:
    OrthographicCamera();

    /**
     * @brief Configures the bounds of the orthographic frustum.
     */
    void set_bounds(float left, float right, float bottom, float top);

    /**
     * @brief Configures near and far clip planes.
     */
    void set_clip_planes(float near_plane, float far_plane);

    /**
     * @brief Updates both bounds and clip planes in one call.
     */
    void set_orthographic(float left, float right, float bottom, float top, float near_plane, float far_plane);

    /**
     * @brief Builds a projection matrix for the current parameters.
     */
    Mat4f projection_matrix() const;

  private:
    float left_;
    float right_;
    float bottom_;
    float top_;
    float near_plane_;
    float far_plane_;
};
