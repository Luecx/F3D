#include "LitShader.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

LitShader::LitShader()
    : view_location_(-1), projection_location_(-1), camera_pos_location_(-1), debug_mode_location_(-1),
      directional_light_count_location_(-1), spot_light_count_location_(-1), point_light_count_location_(-1) {}

bool LitShader::init(const std::filesystem::path& shader_dir) {
    vertex_file((shader_dir / "lit" / "lit.vert").string());
    fragment_file((shader_dir / "lit" / "lit.frag").string());
    compile();
    get_all_uniform_locations();
    return true;
}

void LitShader::get_all_uniform_locations() {
    view_location_ = get_uniform_location("u_view");
    projection_location_ = get_uniform_location("u_projection");
    camera_pos_location_ = get_uniform_location("u_camera_pos");
    debug_mode_location_ = get_uniform_location("u_debug_mode");

    directional_light_count_location_ = get_uniform_location("u_directional_light_count");
    spot_light_count_location_ = get_uniform_location("u_spot_light_count");
    point_light_count_location_ = get_uniform_location("u_point_light_count");

    directional_light_uniforms_.clear();
    for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i) {
        DirectionalLightUniforms uniforms{};
        std::string prefix = "u_directional_lights[" + std::to_string(i) + "]";
        uniforms.color = get_uniform_location(prefix + ".color");
        uniforms.intensity = get_uniform_location(prefix + ".intensity");
        uniforms.direction = get_uniform_location(prefix + ".direction");
        uniforms.casts_shadows = get_uniform_location(prefix + ".casts_shadows");
        uniforms.view_projection = get_uniform_location(prefix + ".light_view_projection");
        uniforms.shadow_sampler = get_uniform_location("u_directional_shadow_maps[" + std::to_string(i) + "]");
        directional_light_uniforms_.push_back(uniforms);
    }

    spot_light_uniforms_.clear();
    for (int i = 0; i < MAX_SPOT_LIGHTS; ++i) {
        SpotLightUniforms uniforms{};
        std::string prefix = "u_spot_lights[" + std::to_string(i) + "]";
        uniforms.color = get_uniform_location(prefix + ".color");
        uniforms.intensity = get_uniform_location(prefix + ".intensity");
        uniforms.position = get_uniform_location(prefix + ".position");
        uniforms.direction = get_uniform_location(prefix + ".direction");
        uniforms.range = get_uniform_location(prefix + ".range");
        uniforms.cos_inner = get_uniform_location(prefix + ".cos_inner");
        uniforms.cos_outer = get_uniform_location(prefix + ".cos_outer");
        uniforms.casts_shadows = get_uniform_location(prefix + ".casts_shadows");
        uniforms.view_projection = get_uniform_location(prefix + ".light_view_projection");
        uniforms.shadow_sampler = get_uniform_location("u_spot_shadow_maps[" + std::to_string(i) + "]");
        spot_light_uniforms_.push_back(uniforms);
    }

    point_light_uniforms_.clear();
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        PointLightUniforms uniforms{};
        std::string prefix = "u_point_lights[" + std::to_string(i) + "]";
        uniforms.color = get_uniform_location(prefix + ".color");
        uniforms.intensity = get_uniform_location(prefix + ".intensity");
        uniforms.position = get_uniform_location(prefix + ".position");
        uniforms.radius = get_uniform_location(prefix + ".radius");
        uniforms.casts_shadows = get_uniform_location(prefix + ".casts_shadows");
        uniforms.shadow_far = get_uniform_location(prefix + ".shadow_far");
        uniforms.shadow_sampler = get_uniform_location("u_point_shadow_maps[" + std::to_string(i) + "]");
        point_light_uniforms_.push_back(uniforms);
    }
}

void LitShader::set_camera_matrices(const Mat4f& view, const Mat4f& projection) {
    load_matrix(view_location_, const_cast<Mat4f&>(view));
    load_matrix(projection_location_, const_cast<Mat4f&>(projection));
}

void LitShader::set_camera_position(const Vec3f& position) {
    if (camera_pos_location_ >= 0) {
        glUniform3f(camera_pos_location_, position[0], position[1], position[2]);
    }
}

void LitShader::set_debug_mode(int mode) {
    if (debug_mode_location_ >= 0) {
        glUniform1i(debug_mode_location_, mode);
    }
}

void LitShader::set_directional_lights(const std::vector<std::pair<DirectionalLight*, Transformation*>>& lights) {
    int count = static_cast<int>(std::min<std::size_t>(lights.size(), MAX_DIRECTIONAL_LIGHTS));
    if (directional_light_count_location_ >= 0) {
        glUniform1i(directional_light_count_location_, count);
    }
    for (int i = 0; i < count; ++i) {
        auto* light = lights[i].first;
        auto* transform = lights[i].second;
        if (!light) {
            continue;
        }
        Vec3f dir = light->direction(transform);
        const auto& uniforms = directional_light_uniforms_[i];
        if (uniforms.color >= 0) {
            glUniform3f(uniforms.color, light->color[0], light->color[1], light->color[2]);
        }
        if (uniforms.intensity >= 0) {
            glUniform1f(uniforms.intensity, light->intensity);
        }
        if (uniforms.direction >= 0) {
            glUniform3f(uniforms.direction, dir[0], dir[1], dir[2]);
        }
        if (uniforms.casts_shadows >= 0) {
            glUniform1i(uniforms.casts_shadows, light->casts_shadows ? 1 : 0);
        }
        if (uniforms.view_projection >= 0) {
            Mat4f vp = light->light_view_projection;
            load_matrix(uniforms.view_projection, vp);
        }
    }
}

void LitShader::set_spot_lights(const std::vector<std::pair<SpotLight*, Transformation*>>& lights) {
    int count = static_cast<int>(std::min<std::size_t>(lights.size(), MAX_SPOT_LIGHTS));
    if (spot_light_count_location_ >= 0) {
        glUniform1i(spot_light_count_location_, count);
    }
    for (int i = 0; i < count; ++i) {
        auto* light = lights[i].first;
        auto* transform = lights[i].second;
        if (!light) {
            continue;
        }
        Vec3f pos = light->position(transform);
        Vec3f dir = light->direction(transform);
        const auto& uniforms = spot_light_uniforms_[i];
        if (uniforms.color >= 0) {
            glUniform3f(uniforms.color, light->color[0], light->color[1], light->color[2]);
        }
        if (uniforms.intensity >= 0) {
            glUniform1f(uniforms.intensity, light->intensity);
        }
        if (uniforms.position >= 0) {
            glUniform3f(uniforms.position, pos[0], pos[1], pos[2]);
        }
        if (uniforms.direction >= 0) {
            glUniform3f(uniforms.direction, dir[0], dir[1], dir[2]);
        }
        if (uniforms.range >= 0) {
            glUniform1f(uniforms.range, light->range);
        }
        constexpr float kPi = 3.14159265359f;
        if (uniforms.cos_inner >= 0) {
            glUniform1f(uniforms.cos_inner, std::cos(light->inner_angle_deg * kPi / 180.0f));
        }
        if (uniforms.cos_outer >= 0) {
            glUniform1f(uniforms.cos_outer, std::cos(light->outer_angle_deg * kPi / 180.0f));
        }
        if (uniforms.casts_shadows >= 0) {
            glUniform1i(uniforms.casts_shadows, light->casts_shadows ? 1 : 0);
        }
        if (uniforms.view_projection >= 0) {
            Mat4f vp = light->light_view_projection;
            load_matrix(uniforms.view_projection, vp);
        }
    }
}

void LitShader::set_point_lights(const std::vector<std::pair<PointLight*, Transformation*>>& lights) {
    int count = static_cast<int>(std::min<std::size_t>(lights.size(), MAX_POINT_LIGHTS));
    if (point_light_count_location_ >= 0) {
        glUniform1i(point_light_count_location_, count);
    }
    for (int i = 0; i < count; ++i) {
        auto* light = lights[i].first;
        auto* transform = lights[i].second;
        if (!light) {
            continue;
        }
        Vec3f position = light->position(transform);
        const auto& uniforms = point_light_uniforms_[i];
        if (uniforms.color >= 0) {
            glUniform3f(uniforms.color, light->color[0], light->color[1], light->color[2]);
        }
        if (uniforms.intensity >= 0) {
            glUniform1f(uniforms.intensity, light->intensity);
        }
        if (uniforms.position >= 0) {
            glUniform3f(uniforms.position, position[0], position[1], position[2]);
        }
        if (uniforms.radius >= 0) {
            glUniform1f(uniforms.radius, light->radius);
        }
        if (uniforms.casts_shadows >= 0) {
            glUniform1i(uniforms.casts_shadows, light->casts_shadows ? 1 : 0);
        }
        if (uniforms.shadow_far >= 0) {
            float far_plane = light->shadow_far > light->shadow_near ? light->shadow_far : light->radius;
            glUniform1f(uniforms.shadow_far, far_plane);
        }
    }
}

void LitShader::bind_directional_shadow_map(int index, GLuint texture_id, int texture_unit) {
    if (index < 0 || index >= static_cast<int>(directional_light_uniforms_.size())) {
        return;
    }
    const auto& uniforms = directional_light_uniforms_[index];
    if (uniforms.shadow_sampler < 0) {
        return;
    }
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glUniform1i(uniforms.shadow_sampler, texture_unit);
}

void LitShader::bind_spot_shadow_map(int index, GLuint texture_id, int texture_unit) {
    if (index < 0 || index >= static_cast<int>(spot_light_uniforms_.size())) {
        return;
    }
    const auto& uniforms = spot_light_uniforms_[index];
    if (uniforms.shadow_sampler < 0) {
        return;
    }
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glUniform1i(uniforms.shadow_sampler, texture_unit);
}

void LitShader::bind_point_shadow_map(int index, GLuint texture_id, int texture_unit) {
    if (index < 0 || index >= static_cast<int>(point_light_uniforms_.size())) {
        return;
    }
    const auto& uniforms = point_light_uniforms_[index];
    if (uniforms.shadow_sampler < 0) {
        return;
    }
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glUniform1i(uniforms.shadow_sampler, texture_unit);
}
