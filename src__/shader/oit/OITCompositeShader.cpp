#include "OITCompositeShader.h"

OITCompositeShader::OITCompositeShader()
    : accum_location_(-1)
    , reveal_location_(-1)
    , opaque_location_(-1) {}

bool OITCompositeShader::init(const std::filesystem::path& shader_dir) {
    vertex_file((shader_dir / "oit" / "oit_composite.vert").string());
    fragment_file((shader_dir / "oit" / "oit_composite.frag").string());
    compile();
    get_all_uniform_locations();
    start();
    set_accum_texture(0);
    set_reveal_texture(1);
    set_opaque_texture(2);
    stop();
    return true;
}

void OITCompositeShader::get_all_uniform_locations() {
    accum_location_ = get_uniform_location("u_transparent_accum");
    reveal_location_ = get_uniform_location("u_transparent_reveal");
    opaque_location_ = get_uniform_location("u_opaque_color");
}

void OITCompositeShader::set_accum_texture(int unit) {
    if (accum_location_ >= 0) {
        glUniform1i(accum_location_, unit);
    }
}

void OITCompositeShader::set_reveal_texture(int unit) {
    if (reveal_location_ >= 0) {
        glUniform1i(reveal_location_, unit);
    }
}

void OITCompositeShader::set_opaque_texture(int unit) {
    if (opaque_location_ >= 0) {
        glUniform1i(opaque_location_, unit);
    }
}
