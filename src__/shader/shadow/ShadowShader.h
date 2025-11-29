#pragma once

#include "../ShaderProgram.h"

#include <filesystem>

#include "../../math/mat.h"

class ShadowShader : public ShaderProgram {
  public:
    ShadowShader();

    bool init(const std::filesystem::path& shader_dir);

    void set_light_vp(const Mat4f& vp);
    void set_point_shadow_params(bool enabled, const Vec3f& light_pos, float far_plane);

  protected:
    void get_all_uniform_locations() override;

  private:
    GLint light_vp_location_;
    GLint light_pos_location_;
    GLint far_plane_location_;
    GLint is_point_light_location_;
};
