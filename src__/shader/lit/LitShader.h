#pragma once

#include "../ShaderProgram.h"

#include <filesystem>
#include <vector>

#include "../../core/config.h"
#include "../../lighting/directional_light.h"
#include "../../lighting/point_light.h"
#include "../../lighting/spot_light.h"
#include "../../math/mat.h"

class Transformation;

class LitShader : public ShaderProgram {
  public:
    LitShader();

    bool init(const std::filesystem::path& shader_dir);

    void set_camera_matrices(const Mat4f& view, const Mat4f& projection);
    void set_camera_position(const Vec3f& position);
    void set_debug_mode(int mode);

    void set_directional_lights(const std::vector<std::pair<DirectionalLight*, Transformation*>>& lights);
    void set_spot_lights(const std::vector<std::pair<SpotLight*, Transformation*>>& lights);
    void set_point_lights(const std::vector<std::pair<PointLight*, Transformation*>>& lights);
    void bind_directional_shadow_map(int index, GLuint texture_id, int texture_unit);
    void bind_spot_shadow_map(int index, GLuint texture_id, int texture_unit);
    void bind_point_shadow_map(int index, GLuint texture_id, int texture_unit);

  protected:
    void get_all_uniform_locations() override;

  private:
    GLint view_location_;
    GLint projection_location_;
    GLint camera_pos_location_;
    GLint debug_mode_location_;

    GLint directional_light_count_location_;
    GLint spot_light_count_location_;
    GLint point_light_count_location_;

    struct DirectionalLightUniforms {
        GLint color;
        GLint intensity;
        GLint direction;
        GLint casts_shadows;
        GLint view_projection;
        GLint shadow_sampler;
    };

    std::vector<DirectionalLightUniforms> directional_light_uniforms_;

    struct SpotLightUniforms {
        GLint color;
        GLint intensity;
        GLint position;
        GLint direction;
        GLint range;
        GLint cos_inner;
        GLint cos_outer;
        GLint casts_shadows;
        GLint view_projection;
        GLint shadow_sampler;
    };

    std::vector<SpotLightUniforms> spot_light_uniforms_;

    struct PointLightUniforms {
        GLint color;
        GLint intensity;
        GLint position;
        GLint radius;
        GLint casts_shadows;
        GLint shadow_far;
        GLint shadow_sampler;
    };

    std::vector<PointLightUniforms> point_light_uniforms_;
};
