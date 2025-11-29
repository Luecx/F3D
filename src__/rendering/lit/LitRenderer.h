#pragma once

#include <filesystem>
#include <utility>
#include <vector>

#include <ecs.h>

#include <glad/glad.h>

#include "../../rendering/RenderScene.h"
#include "../../rendering/InstanceBuffer.h"
#include "../../lighting/directional_light.h"
#include "../../lighting/point_light.h"
#include "../../lighting/spot_light.h"
#include "../../math/transformation.h"
#include "../../gldata/fbo_data.h"
#include "../../shader/lit/LitShader.h"

class LitRenderer {
  public:
    LitRenderer();
    ~LitRenderer();

    bool init(const std::filesystem::path& shader_dir);

    void set_render_target(const FBOData::SPtr& target, int width, int height);

    void render(const RenderableList& renderables, InstanceBuffer& instance_buffer, const Mat4f& view_matrix,
                const Mat4f& projection_matrix, const Vec3f& camera_position,
                const std::vector<std::pair<DirectionalLight*, Transformation*>>& directional_lights,
                const std::vector<std::pair<SpotLight*, Transformation*>>& spot_lights,
                const std::vector<std::pair<PointLight*, Transformation*>>& point_lights);

  private:
    std::unique_ptr<LitShader> shader_;
    FBOData::SPtr target_fbo_;
    int target_width_ {0};
    int target_height_ {0};
};
