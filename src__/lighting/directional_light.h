#pragma once

#include <ecs.h>

#include "../math/mat.h"
#include "../math/transformation.h"
#include "../gldata/fbo_data.h"

class DirectionalLight : public ecs::ComponentOf<DirectionalLight> {
  public:
    DirectionalLight();

    Vec3f color;
    float intensity;
    bool enabled;

    bool casts_shadows;
    float shadow_distance;
    float shadow_extent;
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
