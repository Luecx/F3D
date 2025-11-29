#include "MasterRenderer.h"

#include <chrono>
#include <filesystem>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../camera/camera_controller_system.h"
#include "../camera/orthographic_camera.h"
#include "../camera/perspective_camera.h"
#include "../logging/logging.h"

MasterRenderer::MasterRenderer() = default;

MasterRenderer::~MasterRenderer() { shutdown(); }

bool MasterRenderer::initialize(int width, int height, const std::string& title) {

    logging::log(0, logging::INFO, "Initializing Master Renderer");
    glfwSetErrorCallback([](int code, const char* desc) {
        logging::log(0, logging::ERROR,
                     "GLFW error " + std::to_string(code) + ": " +
                         (desc ? std::string(desc) : std::string("<unknown>")));
    });

    logging::log(0, logging::INFO, "Initializing GLFW");
    if (!glfwInit()) {
        logging::log(1, logging::ERROR, "Failed to initialize GLFW.");
        return false;
    }

    viewport_width_ = width;
    viewport_height_ = height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    logging::log(0, logging::INFO,
                 "Creating window " + title + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        logging::log(1, logging::ERROR, "Failed to create GLFW window.");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window_);

    logging::log(0, logging::INFO, "Loading OpenGL via GLAD");
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        logging::log(1, logging::ERROR, "Failed to initialize GLAD");
        shutdown();
        return false;
    }

    std::filesystem::path shader_dir = std::filesystem::current_path() / "src" / "shader";
    if (!std::filesystem::exists(shader_dir)) {
        shader_dir = std::filesystem::current_path().parent_path() / "src" / "shader";
    }
    shader_dir = std::filesystem::weakly_canonical(shader_dir);

    logging::log(0, logging::INFO, "Loading shaders from " + shader_dir.string());
    lit_renderer_ = std::make_unique<LitRenderer>();
    shadow_renderer_ = std::make_unique<ShadowRenderer>();
    transparent_renderer_ = std::make_unique<TransparentRenderer>();
    oit_renderer_ = std::make_unique<OITRenderer>();

    if (!lit_renderer_->init(shader_dir)) {
        logging::log(0, logging::ERROR, "Failed to initialize lit renderer");
        return false;
    }
    if (!shadow_renderer_->init(shader_dir)) {
        logging::log(0, logging::ERROR, "Failed to initialize shadow renderer");
        return false;
    }
    if (!transparent_renderer_->init(shader_dir)) {
        logging::log(0, logging::ERROR, "Failed to initialize transparent renderer");
        return false;
    }
    if (!oit_renderer_->initialize(shader_dir, viewport_width_, viewport_height_)) {
        logging::log(0, logging::ERROR, "Failed to initialize OIT renderer");
        return false;
    }

    ecs_.create_system<CameraControllerSystem>(window_);

    glEnable(GL_DEPTH_TEST);

    return true;
}

void MasterRenderer::shutdown() {
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
    }
}

void MasterRenderer::set_active_camera(ecs::EntityID camera_id) { active_camera_ = camera_id; }

void MasterRenderer::run() {
    auto last_frame = std::chrono::steady_clock::now();

    while (window_ && !glfwWindowShouldClose(window_)) {
        auto now = std::chrono::steady_clock::now();
        float delta_seconds = std::chrono::duration<float>(now - last_frame).count();
        last_frame = now;

        int fb_width = viewport_width_;
        int fb_height = viewport_height_;
        glfwGetFramebufferSize(window_, &fb_width, &fb_height);
        if (fb_width != viewport_width_ || fb_height != viewport_height_) {
            viewport_width_ = fb_width;
            viewport_height_ = fb_height;
            oit_renderer_->resize(viewport_width_, viewport_height_);
        }

        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);

        Mat4f view_matrix = Mat4f::eye();
        Mat4f projection_matrix = Mat4f::eye();
        Vec3f camera_position{0.0f, 0.0f, 0.0f};
        bool camera_valid = false;
        if (active_camera_.id != ecs::INVALID_ID) {
            auto& entity = ecs_[active_camera_.id];
            if (auto* transform = entity.get<Transformation>()) {
                view_matrix = transform->global_matrix().inverse();
                camera_position = transform->global_position();
                camera_valid = true;
            }

            if (auto* perspective = entity.get<PerspectiveCamera>()) {
                projection_matrix = perspective->projection_matrix();
            } else if (auto* orthographic = entity.get<OrthographicCamera>()) {
                projection_matrix = orthographic->projection_matrix();
            } else {
                logging::log(0, logging::WARNING,
                             "MasterRenderer: active camera lacks a projection component (entity " +
                                 std::to_string(active_camera_.id) + ")");
            }
        }

        if (!camera_valid) {
            logging::log(0, logging::WARNING,
                         "MasterRenderer: active camera invalid (entity " + std::to_string(active_camera_.id) + ")");
        } else {
            logging::log(0, logging::DEBUG,
                         "MasterRenderer: using active camera entity " + std::to_string(active_camera_.id));
        }

        auto directional_lights = gather_directional_lights();
        auto spot_lights = gather_spot_lights();
        auto point_lights = gather_point_lights();
        const auto& renderables = gather_renderables();
        instance_buffer_.sync(renderables);
        logging::log(0, logging::DEBUG,
                     "MasterRenderer: gathered " + std::to_string(directional_lights.size()) + " directional, " +
                         std::to_string(spot_lights.size()) + " spot and " + std::to_string(point_lights.size()) +
                         " point lights");

        if (shadow_renderer_) {
            logging::log(0, logging::DEBUG, "MasterRenderer: invoking shadow renderer");
            shadow_renderer_->render(renderables, instance_buffer_, directional_lights, spot_lights, point_lights);
            logging::log(0, logging::DEBUG, "MasterRenderer: finished shadow pass");
        }

        oit_renderer_->prepare_opaque_target();

        if (lit_renderer_) {
            lit_renderer_->set_render_target(oit_renderer_->opaque_fbo(), viewport_width_, viewport_height_);
            logging::log(0, logging::DEBUG, "MasterRenderer: invoking lit renderer");
            lit_renderer_->render(renderables, instance_buffer_, view_matrix, projection_matrix, camera_position,
                                  directional_lights, spot_lights, point_lights);
            logging::log(0, logging::DEBUG, "MasterRenderer: finished lit pass");
        }

        bool has_transparent_instances = false;
        for (const auto& renderable : renderables) {
            if (renderable.transparent) {
                has_transparent_instances = true;
                break;
            }
        }

        oit_renderer_->prepare_transparent_target();
        if (transparent_renderer_) {
            transparent_renderer_->set_render_target(oit_renderer_->transparent_fbo(),
                                                     viewport_width_,
                                                     viewport_height_);
        }
        if (has_transparent_instances && transparent_renderer_) {
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunci(0, GL_ONE, GL_ONE);
            glBlendEquationi(0, GL_FUNC_ADD);
            glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
            glBlendEquationi(1, GL_FUNC_ADD);

            transparent_renderer_->render(renderables,
                                          instance_buffer_,
                                          view_matrix,
                                          projection_matrix,
                                          camera_position,
                                          directional_lights,
                                          spot_lights,
                                          point_lights);

            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
        }

        oit_renderer_->composite(viewport_width_, viewport_height_);

        glfwSwapBuffers(window_);
        glfwPollEvents();
        ecs_.process(delta_seconds);
    }
}

const RenderableList& MasterRenderer::gather_renderables() {
    renderables_.clear();
    for (auto& entity : ecs_.each<ModelComponent>()) {
        auto* model = entity.get<ModelComponent>();
        auto* instances = entity.get<Instances>();
        auto* visibility = entity.get<Visibility>();
        auto* shadow = entity.get<ShadowCaster>();
        auto* transparency_component = entity.get<Transparency>();
        if (!model || !model->valid() || !instances || instances->count() == 0) {
            continue;
        }
        if (visibility && !visibility->enabled) {
            continue;
        }
        bool transparent = transparency_component ? transparency_component->enabled : false;
        if (!transparent && model && model->mesh) {
            switch (model->transparency_mode) {
            case TransparencyMode::ForceTransparent:
                transparent = true;
                break;
            case TransparencyMode::ForceOpaque:
                transparent = false;
                break;
            case TransparencyMode::Auto:
            default:
                transparent = model->mesh->has_transparent_materials();
                break;
            }
        }
        renderables_.push_back(
            RenderableInstance{model, instances, visibility, shadow, transparency_component, transparent});
    }
    logging::log(0, logging::DEBUG,
                 "MasterRenderer: gathered " + std::to_string(renderables_.size()) + " renderable entries");
    return renderables_;
}

MasterRenderer::DirectionalLightList MasterRenderer::gather_directional_lights() {
    DirectionalLightList lights;
    for (auto& entity : ecs_.each<DirectionalLight>()) {
        auto* light = entity.get<DirectionalLight>();
        auto* transform = entity.get<Transformation>();
        auto* visible = entity.get<Visible>();
        if (!light || !transform || (visible && !visible->enabled) || !light->enabled) {
            continue;
        }
        light->update_matrices(transform);
        if (light->casts_shadows) {
            light->ensure_shadow_resources();
        }
        lights.emplace_back(light, transform);
    }
    return lights;
}

MasterRenderer::PointLightList MasterRenderer::gather_point_lights() {
    PointLightList lights;
    for (auto& entity : ecs_.each<PointLight>()) {
        auto* light = entity.get<PointLight>();
        auto* transform = entity.get<Transformation>();
        auto* visible = entity.get<Visible>();
        if (!light || !transform || (visible && !visible->enabled) || !light->enabled) {
            continue;
        }
        light->update_shadow_matrices(transform);
        if (light->casts_shadows) {
            light->ensure_shadow_resources();
        }
        lights.emplace_back(light, transform);
    }
    return lights;
}

MasterRenderer::SpotLightList MasterRenderer::gather_spot_lights() {
    SpotLightList lights;
    for (auto& entity : ecs_.each<SpotLight>()) {
        auto* light = entity.get<SpotLight>();
        auto* transform = entity.get<Transformation>();
        auto* visible = entity.get<Visible>();
        if (!light || !transform || (visible && !visible->enabled) || !light->enabled) {
            continue;
        }
        light->update_matrices(transform);
        if (light->casts_shadows) {
            light->ensure_shadow_resources();
        }
        lights.emplace_back(light, transform);
    }
    return lights;
}
