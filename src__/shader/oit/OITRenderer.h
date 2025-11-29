#pragma once

#include <filesystem>
#include <memory>

#include <glad/glad.h>

#include "../oit/OITCompositeShader.h"
#include "../../gldata/fbo_data.h"
#include "../../gldata/texture_data.h"

class OITRenderer {
public:
    OITRenderer();
    ~OITRenderer();

    bool initialize(const std::filesystem::path& shader_dir, int width, int height);
    void resize(int width, int height);

    void prepare_opaque_target();
    void prepare_transparent_target();
    void resolve_opaque_to_backbuffer(int width, int height);
    void composite(int width, int height);

    const FBOData::SPtr& opaque_fbo() const { return main_fbo_; }
    const FBOData::SPtr& transparent_fbo() const { return oit_fbo_; }
    const TextureData::SPtr& opaque_color_texture() const { return main_color_tex_; }
    const TextureData::SPtr& transparent_accum_texture() const { return oit_accum_tex_; }
    const TextureData::SPtr& transparent_reveal_texture() const { return oit_reveal_tex_; }

private:
    bool create_targets(int width, int height);
    void destroy_targets();

    OITCompositeShader composite_shader_;

    FBOData::SPtr main_fbo_;
    FBOData::SPtr oit_fbo_;
    TextureData::SPtr main_color_tex_;
    TextureData::SPtr depth_tex_;
    TextureData::SPtr oit_accum_tex_;
    TextureData::SPtr oit_reveal_tex_;

    int width_ {0};
    int height_ {0};
};
