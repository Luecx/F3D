#include "LitTransparentShader.h"

LitTransparentShader::LitTransparentShader() = default;

bool LitTransparentShader::init(const std::filesystem::path& shader_dir) {
    vertex_file((shader_dir / "lit" / "lit.vert").string());
    fragment_file((shader_dir / "lit" / "lit_transparent.frag").string());
    compile();
    get_all_uniform_locations();
    return true;
}
