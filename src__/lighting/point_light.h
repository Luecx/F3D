#pragma once

#include <ecs.h>

#include "../gldata/fbo_data.h"
#include "../math/mat.h"
#include "../math/transformation.h"

class PointLight : public ecs::ComponentOf<PointLight> {
  public:
    PointLight();

    Vec3f color;
    float intensity;
    float radius;
    bool enabled;
    bool casts_shadows;
    float shadow_bias;
    float shadow_near;
    float shadow_far;
    int shadow_resolution;

    Mat4f shadow_matrices[6];
    std::unique_ptr<FBOData> shadow_map;
    int current_shadow_resolution;

    Vec3f position(const Transformation* transform) const;
    void update_shadow_matrices(const Transformation* transform);
    void ensure_shadow_resources();
};
