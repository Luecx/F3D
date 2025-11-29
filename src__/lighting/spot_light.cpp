#include "spot_light.h"

#include <cmath>

#include <glad/glad.h>

SpotLight::SpotLight()
    : color{1.0f, 1.0f, 1.0f}
    , intensity(2.0f)
    , range(25.0f)
    , enabled(true)
    , casts_shadows(true)
    , inner_angle_deg(20.0f)
    , outer_angle_deg(30.0f)
    , shadow_bias(0.002f)
    , shadow_resolution(1024)
    , current_shadow_resolution(0) {}

Vec3f SpotLight::direction(const Transformation* transform) const {
    if (transform) {
        return -(transform->global_zaxis().normalised());
    }
    return {0.0f, -1.0f, 0.0f};
}

Vec3f SpotLight::position(const Transformation* transform) const {
    if (transform) {
        return transform->global_position();
    }
    return {0.0f, 0.0f, 0.0f};
}

void SpotLight::update_matrices(const Transformation* transform) {
    Vec3f pos = position(transform);
    Vec3f dir = direction(transform);
    Vec3f target = pos + dir;
    Vec3f up{0.0f, 1.0f, 0.0f};
    if (std::abs(dir.dot(up)) > 0.99f) {
        up = {0.0f, 0.0f, 1.0f};
    }

    light_view = Mat4f::eye().view_look_at(pos, target, up);
    float fov = std::max(outer_angle_deg * 2.0f, inner_angle_deg * 2.0f);
    light_projection = Mat4f::eye().view_perspective(fov, 1.0f, 0.1f, range);
    light_view_projection = light_projection.matmul(light_view);
}

void SpotLight::ensure_shadow_resources() {
    if (!casts_shadows) {
        shadow_map.reset();
        current_shadow_resolution = 0;
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
