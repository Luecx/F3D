/**
 * @file perspective_camera.h
 * @brief Declares the PerspectiveCamera component responsible for perspective projection data.
 */

#pragma once

#include "../math/mat.h"
#include "../math/transformation.h"

/**
 * @brief Perspective projection camera component.
 */
class PerspectiveCamera : public ecs::ComponentOf<PerspectiveCamera> {
  public:
    PerspectiveCamera();

    /**
     * @brief Configures the lens properties of the camera.
     * @param fov_deg Vertical field of view in degrees.
     * @param near_plane Near clip plane distance.
     * @param far_plane Far clip plane distance.
     */
    void set_perspective(float fov_deg, float near_plane, float far_plane);

    /**
     * @brief Updates the viewport size used to derive the aspect ratio.
     * @param width Viewport width in pixels.
     * @param height Viewport height in pixels.
     */
    void set_viewport(int width, int height);

    /**
     * @brief Builds a perspective projection matrix for the current parameters.
     * @return Newly built projection matrix.
     */
    Mat4f projection_matrix() const;

    /**
     * @brief Returns the current viewport width.
     */
    int viewport_width() const { return viewport_width_; }

    /**
     * @brief Returns the current viewport height.
     */
    int viewport_height() const { return viewport_height_; }

  private:
    float aspect_ratio() const;

    float fov_deg_;
    float near_plane_;
    float far_plane_;
    int viewport_width_;
    int viewport_height_;
};
