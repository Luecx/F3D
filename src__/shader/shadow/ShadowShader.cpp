#include "ShadowShader.h"

#include <glad/glad.h>

ShadowShader::ShadowShader()
    : light_vp_location_(-1), light_pos_location_(-1), far_plane_location_(-1), is_point_light_location_(-1) {}

bool ShadowShader::init(const std::filesystem::path& shader_dir) {
    vertex_file((shader_dir / "shadow" / "shadow_depth.vert").string());
    fragment_file((shader_dir / "shadow" / "shadow_depth.frag").string());
    compile();
    get_all_uniform_locations();
    return true;
}

void ShadowShader::get_all_uniform_locations() {
    light_vp_location_ = get_uniform_location("u_light_vp");
    light_pos_location_ = get_uniform_location("u_light_pos");
    far_plane_location_ = get_uniform_location("u_far_plane");
    is_point_light_location_ = get_uniform_location("u_is_point_light");
}

void ShadowShader::set_light_vp(const Mat4f& vp) { load_matrix(light_vp_location_, const_cast<Mat4f&>(vp)); }

void ShadowShader::set_point_shadow_params(bool enabled, const Vec3f& light_pos, float far_plane) {
    if (is_point_light_location_ >= 0) {
        glUniform1i(is_point_light_location_, enabled ? 1 : 0);
    }
    if (light_pos_location_ >= 0) {
        glUniform3f(light_pos_location_, light_pos[0], light_pos[1], light_pos[2]);
    }
    if (far_plane_location_ >= 0) {
        glUniform1f(far_plane_location_, far_plane);
    }
}
