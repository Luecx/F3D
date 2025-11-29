#pragma once

#include "../ShaderProgram.h"

#include <filesystem>

class OITCompositeShader : public ShaderProgram {
public:
    OITCompositeShader();

    bool init(const std::filesystem::path& shader_dir);

    void set_accum_texture(int unit);
    void set_reveal_texture(int unit);
    void set_opaque_texture(int unit);

protected:
    void get_all_uniform_locations() override;

private:
    GLint accum_location_;
    GLint reveal_location_;
    GLint opaque_location_;
};
