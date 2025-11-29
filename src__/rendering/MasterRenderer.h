#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include <ecs.h>

struct GLFWwindow;

#include "components/Visible.h"
#include "components/ModelComponent.h"
#include "components/ShadowCaster.h"
#include "components/Instances.h"
#include "components/Transparency.h"
#include "../lighting/directional_light.h"
#include "../lighting/point_light.h"
#include "../lighting/spot_light.h"
#include "../math/transformation.h"
#include "../resources/resource_manager.h"
#include "lit/LitRenderer.h"
#include "../shader/oit/OITRenderer.h"
#include "../shader/shadow/ShadowRenderer.h"
#include "lit_transparent/TransparentRenderer.h"
#include "InstanceBuffer.h"
#include "RenderScene.h"

class MasterRenderer {
  public:
    MasterRenderer();
    ~MasterRenderer();

    bool initialize(int width, int height, const std::string& title);
    void shutdown();

    ecs::ECS& ecs() { return ecs_; }
    ResourceManager& resources() { return resource_manager_; }

    void set_active_camera(ecs::EntityID camera_id);

    void run();

  private:
    using DirectionalLightList = std::vector<std::pair<DirectionalLight*, Transformation*>>;
    using PointLightList = std::vector<std::pair<PointLight*, Transformation*>>;
    using SpotLightList = std::vector<std::pair<SpotLight*, Transformation*>>;

    DirectionalLightList gather_directional_lights();
    PointLightList gather_point_lights();
    SpotLightList gather_spot_lights();
    const RenderableList& gather_renderables();

    GLFWwindow* window_{nullptr};
    int viewport_width_{0};
    int viewport_height_{0};

    ResourceManager resource_manager_;
    ecs::ECS ecs_;

    std::unique_ptr<LitRenderer> lit_renderer_;
    std::unique_ptr<ShadowRenderer> shadow_renderer_;
    std::unique_ptr<TransparentRenderer> transparent_renderer_;
    std::unique_ptr<OITRenderer> oit_renderer_;

    InstanceBuffer instance_buffer_;
    RenderableList renderables_;

    ecs::EntityID active_camera_{ecs::EntityID{ecs::INVALID_ID}};
};
