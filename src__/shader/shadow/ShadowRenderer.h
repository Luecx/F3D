#pragma once

#include <filesystem>
#include <utility>
#include <vector>

#include "../../rendering/RenderScene.h"
#include "../../rendering/InstanceBuffer.h"
#include "../../rendering/RenderBatchBuilder.h"
#include "../../lighting/directional_light.h"
#include "../../lighting/spot_light.h"
#include "../../lighting/point_light.h"
#include "../../math/transformation.h"
#include "ShadowShader.h"

class ShadowRenderer {
  public:
    ShadowRenderer();
    ~ShadowRenderer();

    bool init(const std::filesystem::path& shader_dir);

    void render(const RenderableList& renderables, InstanceBuffer& instance_buffer,
                const std::vector<std::pair<DirectionalLight*, Transformation*>>& directional_lights,
                const std::vector<std::pair<SpotLight*, Transformation*>>& spot_lights,
                const std::vector<std::pair<PointLight*, Transformation*>>& point_lights);

  private:
    std::unique_ptr<ShadowShader> shader_;
};
