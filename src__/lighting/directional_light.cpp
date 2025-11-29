#include "directional_light.h"

#include <algorithm>
#include <cmath>

#include <glad/glad.h>

DirectionalLight::DirectionalLight()
    : color{1.0f, 1.0f, 1.0f}, intensity(1.0f), enabled(true), casts_shadows(true), shadow_distance(50.0f),
      shadow_extent(25.0f), shadow_resolution(1024), light_view(Mat4f::eye()), light_projection(Mat4f::eye()),
      light_view_projection(Mat4f::eye()), current_shadow_resolution(0) {}

Vec3f DirectionalLight::direction(const Transformation* transform) const {
    if (transform) {
        return -(transform->global_zaxis().normalised());
    }
    return {0.0f, -1.0f, 0.0f};
}

Vec3f DirectionalLight::position(const Transformation* transform) const {
    if (transform) {
        return transform->global_position();
    }
    return {0.0f, 0.0f, 0.0f};
}

void DirectionalLight::update_matrices(const Transformation* transform) {
    Vec3f dir = direction(transform);
    Vec3f pos = position(transform);

    Vec3f target = pos + dir;
    Vec3f up{0.0f, 1.0f, 0.0f};
    if (std::abs(dir.dot(up)) > 0.99f) {
        up = {0.0f, 0.0f, 1.0f};
    }

    light_view = Mat4f::eye().view_look_at(pos, target, up);
    light_projection = Mat4f::eye().view_orthogonal(-shadow_extent, shadow_extent, -shadow_extent, shadow_extent, 0.1f,
                                                    shadow_distance);
    light_view_projection = light_projection.matmul(light_view);
}

void DirectionalLight::ensure_shadow_resources() {
    if (!casts_shadows) {
        shadow_map.reset();
        return;
    }

    if (!shadow_map) {
        shadow_map = std::make_unique<FBOData>(TextureType::TEX_2D);
        current_shadow_resolution = 0;
    }
    if (current_shadow_resolution != shadow_resolution) {
        TextureSpecification depth_spec;
        depth_spec.type = TextureType::TEX_2D;
        depth_spec.internal_format = GL_DEPTH_COMPONENT32F;
        depth_spec.data_format = GL_DEPTH_COMPONENT;
        depth_spec.data_type = GL_FLOAT;
        depth_spec.min_filter = GL_LINEAR;
        depth_spec.mag_filter = GL_LINEAR;
        depth_spec.wrap_s = depth_spec.wrap_t = depth_spec.wrap_r = GL_CLAMP_TO_EDGE;
        depth_spec.generate_mipmaps = false;
        shadow_map->create_depth_attachment(shadow_resolution, shadow_resolution, depth_spec);
        current_shadow_resolution = shadow_resolution;
    }
}
