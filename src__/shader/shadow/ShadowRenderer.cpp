#include "ShadowRenderer.h"

#include <glad/glad.h>

#include "../../logging/logging.h"
#include "../../math/mat.h"

ShadowRenderer::ShadowRenderer() : shader_(std::make_unique<ShadowShader>()) {}

ShadowRenderer::~ShadowRenderer() = default;

bool ShadowRenderer::init(const std::filesystem::path& shader_dir) { return shader_->init(shader_dir); }

void ShadowRenderer::render(const RenderableList& renderables, InstanceBuffer& instance_buffer,
                            const std::vector<std::pair<DirectionalLight*, Transformation*>>& directional_lights,
                            const std::vector<std::pair<SpotLight*, Transformation*>>& spot_lights,
                            const std::vector<std::pair<PointLight*, Transformation*>>& point_lights) {
    if (directional_lights.empty() && spot_lights.empty() && point_lights.empty()) {
        return;
    }

    auto batches = build_mesh_batches(renderables, instance_buffer, [](const RenderableInstance& renderable) {
        if (!renderable.model || !renderable.shadow) {
            return false;
        }
        if (!renderable.model->casts_shadows) {
            return false;
        }
        return renderable.shadow->casts_shadows;
    });

    if (batches.empty()) {
        logging::log(0, logging::DEBUG, "ShadowRenderer: no batches to draw");
        return;
    }

    logging::log(0, logging::DEBUG,
                 "ShadowRenderer: prepared " + std::to_string(batches.size()) + " batches over " +
                     std::to_string(instance_buffer.total_instances()) + " instances");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    shader_->start();
    instance_buffer.bind(0);
    for (const auto& entry : directional_lights) {
        auto* light = entry.first;
        if (!light || !light->casts_shadows || !light->shadow_map || !light->shadow_map->depth_texture()) {
            continue;
        }

        light->shadow_map->bind();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glViewport(0, 0, light->shadow_resolution, light->shadow_resolution);
        glClear(GL_DEPTH_BUFFER_BIT);

        shader_->set_point_shadow_params(false, Vec3f{0.0f, 0.0f, 0.0f}, 1.0f);
        shader_->set_light_vp(light->light_view_projection);

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
                    logging::log(
                        0, logging::ERROR,
                        "ShadowRenderer: directional draw exceeds SSBO (base=" + std::to_string(draw.base_instance) +
                            ", count=" + std::to_string(draw.instance_count) +
                            ", total=" + std::to_string(total_instances) + ")");
                    continue;
                }
                batch.mesh->draw_instanced(draw.instance_count, draw.base_instance);
            }
        }

        light->shadow_map->unbind();
    }

    for (const auto& entry : spot_lights) {
        auto* light = entry.first;
        if (!light || !light->casts_shadows || !light->shadow_map || !light->shadow_map->depth_texture()) {
            continue;
        }

        light->shadow_map->bind();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glViewport(0, 0, light->shadow_resolution, light->shadow_resolution);
        glClear(GL_DEPTH_BUFFER_BIT);

        shader_->set_point_shadow_params(false, Vec3f{0.0f, 0.0f, 0.0f}, 1.0f);
        shader_->set_light_vp(light->light_view_projection);

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
                    logging::log(0, logging::ERROR,
                                 "ShadowRenderer: spot draw exceeds SSBO (base=" + std::to_string(draw.base_instance) +
                                     ", count=" + std::to_string(draw.instance_count) +
                                     ", total=" + std::to_string(total_instances) + ")");
                    continue;
                }
                batch.mesh->draw_instanced(draw.instance_count, draw.base_instance);
            }
        }

        light->shadow_map->unbind();
    }

    for (const auto& entry : point_lights) {
        auto* light = entry.first;
        if (!light || !light->casts_shadows || !light->shadow_map || !light->shadow_map->depth_texture()) {
            continue;
        }

        auto* depth_texture = light->shadow_map->depth_texture();
        GLuint texture_id = static_cast<GLuint>(*depth_texture);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        Vec3f light_pos = light->position(entry.second);
        float far_plane = light->shadow_far > light->shadow_near ? light->shadow_far : light->radius;
        if (far_plane <= 0.0f) {
            far_plane = 1.0f;
        }
        shader_->set_point_shadow_params(true, light_pos, far_plane);

        for (int face = 0; face < 6; ++face) {
            light->shadow_map->bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   texture_id, 0);
            glViewport(0, 0, light->shadow_resolution, light->shadow_resolution);
            glClear(GL_DEPTH_BUFFER_BIT);

            Mat4f light_vp = light->shadow_matrices[face];
            shader_->set_light_vp(light_vp);

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
                        logging::log(
                            0, logging::ERROR,
                            "ShadowRenderer: point draw exceeds SSBO (base=" + std::to_string(draw.base_instance) +
                                ", count=" + std::to_string(draw.instance_count) +
                                ", total=" + std::to_string(total_instances) + ")");
                        continue;
                    }
                    batch.mesh->draw_instanced(draw.instance_count, draw.base_instance);
                }
            }

            light->shadow_map->unbind();
        }
    }
    shader_->stop();

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
}
