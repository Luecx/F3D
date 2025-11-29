#pragma once

#include <ecs.h>

#include "../gldata/fbo_data.h"
#include "../math/mat.h"
#include "../math/transformation.h"

class SpotLight : public ecs::ComponentOf<SpotLight> {
  public:
    SpotLight();

    Vec3f color;
    float intensity;
    float range;
    bool enabled;
    bool casts_shadows;
    float inner_angle_deg;
    float outer_angle_deg;
    float shadow_bias;
    int shadow_resolution;

    Mat4f light_view;
    Mat4f light_projection;
    Mat4f light_view_projection;
    std::unique_ptr<FBOData> shadow_map;
    int current_shadow_resolution;

    Vec3f direction(const Transformation* transform) const;
    Vec3f position(const Transformation* transform) const;

    void update_matrices(const Transformation* transform);
    void ensure_shadow_resources();
};
