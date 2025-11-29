#include "point_light.h"

#include "../core/config.h"
#include <glad/glad.h>

PointLight::PointLight()
    : color{1.0f, 1.0f, 1.0f}
    , intensity(1.0f)
    , radius(10.0f)
    , enabled(true)
    , casts_shadows(false)
    , shadow_bias(0.02f)
    , shadow_near(0.1f)
    , shadow_far(25.0f)
    , shadow_resolution(1024)
    , current_shadow_resolution(0) {}

Vec3f PointLight::position(const Transformation* transform) const {
    if (transform) {
        return transform->global_position();
    }
    return {0.0f, 0.0f, 0.0f};
}

void PointLight::update_shadow_matrices(const Transformation* transform) {
    if (!casts_shadows) {
        return;
    }
    Vec3f pos = position(transform);
    float far_plane = shadow_far > shadow_near ? shadow_far : radius;
    Mat4f projection = Mat4f::eye().view_perspective(90.0f, 1.0f, shadow_near, far_plane);

    Vec3f directions[6] = {
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f},
    };

    Vec3f up_vectors[6] = {
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
    };

    for (int i = 0; i < 6; ++i) {
        Mat4f view = Mat4f::eye().view_look_at(pos, pos + directions[i], up_vectors[i]);
        shadow_matrices[i] = projection.matmul(view);
    }
}

void PointLight::ensure_shadow_resources() {
    if (!casts_shadows) {
        shadow_map.reset();
        current_shadow_resolution = 0;
        return;
    }

    if (!shadow_map) {
        shadow_map = std::make_unique<FBOData>(TextureType::TEX_CUBE_MAP);
        current_shadow_resolution = 0;
    }
    if (current_shadow_resolution != shadow_resolution) {
        TextureSpecification depth_spec;
        depth_spec.type = TextureType::TEX_CUBE_MAP;
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
