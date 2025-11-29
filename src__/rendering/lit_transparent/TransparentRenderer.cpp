#include "TransparentRenderer.h"

#include <glad/glad.h>

#include "../../logging/logging.h"
#include "../../core/config.h"
#include "../../rendering/RenderBatchBuilder.h"

TransparentRenderer::TransparentRenderer()
    : shader_(std::make_unique<LitTransparentShader>()) {}

TransparentRenderer::~TransparentRenderer() = default;

bool TransparentRenderer::init(const std::filesystem::path& shader_dir) {
    return shader_->init(shader_dir);
}

void TransparentRenderer::set_render_target(const FBOData::SPtr& target, int width, int height) {
    target_fbo_ = target;
    target_width_ = width;
    target_height_ = height;
}

void TransparentRenderer::render(const RenderableList& renderables,
                                 InstanceBuffer& instance_buffer,
                                 const Mat4f& view_matrix,
                                 const Mat4f& projection_matrix,
                                 const Vec3f& camera_position,
                                 const std::vector<std::pair<DirectionalLight*, Transformation*>>& directional_lights,
                                 const std::vector<std::pair<SpotLight*, Transformation*>>& spot_lights,
                                 const std::vector<std::pair<PointLight*, Transformation*>>& point_lights) {
    GLint previous_fbo = 0;
    GLint previous_viewport[4] = {0, 0, 0, 0};
    GLint previous_draw_buffer = GL_BACK;
    GLint previous_read_buffer = GL_BACK;
    if (target_fbo_) {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_fbo);
        glGetIntegerv(GL_VIEWPORT, previous_viewport);
        glGetIntegerv(GL_DRAW_BUFFER, &previous_draw_buffer);
        glGetIntegerv(GL_READ_BUFFER, &previous_read_buffer);
        GLuint fbo = static_cast<GLuint>(*target_fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, target_width_, target_height_);
        GLenum buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, buffers);
    }

    glEnable(GL_DEPTH_TEST);

    shader_->start();
    shader_->set_camera_matrices(view_matrix, projection_matrix);
    shader_->set_camera_position(camera_position);
    shader_->set_debug_mode(0);
    shader_->set_directional_lights(directional_lights);
    shader_->set_spot_lights(spot_lights);
    shader_->set_point_lights(point_lights);

    int next_texture_unit = 1;
    for (int i = 0; i < static_cast<int>(directional_lights.size()) && i < MAX_DIRECTIONAL_LIGHTS; ++i) {
        auto* light = directional_lights[i].first;
        if (!light || !light->casts_shadows || !light->shadow_map || !light->shadow_map->depth_texture()) {
            continue;
        }
        GLuint depth_id = static_cast<GLuint>(*light->shadow_map->depth_texture());
        shader_->bind_directional_shadow_map(i, depth_id, next_texture_unit);
        next_texture_unit++;
    }
    glActiveTexture(GL_TEXTURE0);

    for (int i = 0; i < static_cast<int>(spot_lights.size()) && i < MAX_SPOT_LIGHTS; ++i) {
        auto* light = spot_lights[i].first;
        if (!light || !light->casts_shadows || !light->shadow_map || !light->shadow_map->depth_texture()) {
            continue;
        }
        GLuint depth_id = static_cast<GLuint>(*light->shadow_map->depth_texture());
        shader_->bind_spot_shadow_map(i, depth_id, next_texture_unit);
        next_texture_unit++;
    }

    for (int i = 0; i < static_cast<int>(point_lights.size()) && i < MAX_POINT_LIGHTS; ++i) {
        auto* light = point_lights[i].first;
        if (!light || !light->casts_shadows || !light->shadow_map || !light->shadow_map->depth_texture()) {
            continue;
        }
        GLuint depth_id = static_cast<GLuint>(*light->shadow_map->depth_texture());
        shader_->bind_point_shadow_map(i, depth_id, next_texture_unit);
        next_texture_unit++;
    }

    instance_buffer.bind(0);

    auto batches = build_mesh_batches(renderables,
                                      instance_buffer,
                                      [](const RenderableInstance& instance) {
                                          return instance.transparent;
                                      });

    int rendered_instances = 0;
    for (const auto& batch : batches) {
        if (!batch.mesh) {
            continue;
        }

        if (batch.double_sided) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }

        for (const auto& draw : batch.draws) {
            if (draw.instance_count <= 0) {
                continue;
            }
            const auto total_instances = instance_buffer.total_instances();
            if (static_cast<std::size_t>(draw.base_instance) + draw.instance_count > total_instances) {
                logging::log(0,
                             logging::ERROR,
                             "TransparentRenderer: draw range exceeds SSBO (base="
                                 + std::to_string(draw.base_instance) + ", count="
                                 + std::to_string(draw.instance_count) + ", total="
                                 + std::to_string(total_instances) + ")");
                continue;
            }
            batch.mesh->draw_instanced(draw.instance_count, draw.base_instance);
            rendered_instances += draw.instance_count;
        }
    }

    shader_->stop();
    glDisable(GL_CULL_FACE);

    logging::log(0,
                 logging::DEBUG,
                 "TransparentRenderer: rendered " + std::to_string(rendered_instances)
                     + " instances across " + std::to_string(batches.size()) + " batches.");

    if (target_fbo_) {
        glBindFramebuffer(GL_FRAMEBUFFER, previous_fbo);
        glDrawBuffer(previous_draw_buffer);
        glReadBuffer(previous_read_buffer);
        glViewport(previous_viewport[0], previous_viewport[1], previous_viewport[2], previous_viewport[3]);
    }
}
