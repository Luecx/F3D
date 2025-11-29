#include "OITRenderer.h"

#include <filesystem>

#include "../../logging/logging.h"

namespace {
TextureSpecification make_texture_spec(GLint internal_format, GLenum format, GLenum type) {
    TextureSpecification spec;
    spec.type = TextureType::TEX_2D;
    spec.internal_format = internal_format;
    spec.data_format = format;
    spec.data_type = type;
    spec.min_filter = GL_LINEAR;
    spec.mag_filter = GL_LINEAR;
    spec.wrap_s = GL_CLAMP_TO_EDGE;
    spec.wrap_t = GL_CLAMP_TO_EDGE;
    spec.wrap_r = GL_CLAMP_TO_EDGE;
    spec.generate_mipmaps = false;
    return spec;
}
}

OITRenderer::OITRenderer() = default;

OITRenderer::~OITRenderer() { destroy_targets(); }

bool OITRenderer::initialize(const std::filesystem::path& shader_dir, int width, int height) {
    if (!composite_shader_.init(shader_dir)) {
        logging::log(0, logging::ERROR, "OITRenderer: failed to initialize composite shader");
        return false;
    }
    width_ = width;
    height_ = height;
    return create_targets(width_, height_);
}

void OITRenderer::resize(int width, int height) {
    if (width == width_ && height == height_) {
        return;
    }
    width_ = width;
    height_ = height;
    create_targets(width_, height_);
}

void OITRenderer::prepare_opaque_target() {
    if (!main_fbo_) {
        return;
    }
    GLuint fbo = static_cast<GLuint>(*main_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width_, height_);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
}

void OITRenderer::prepare_transparent_target() {
    if (!oit_fbo_) {
        return;
    }
    GLuint fbo = static_cast<GLuint>(*oit_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width_, height_);
    GLenum buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);
    const float clear_accum[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const float clear_reveal[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_accum);
    glClearBufferfv(GL_COLOR, 1, clear_reveal);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
}

void OITRenderer::resolve_opaque_to_backbuffer(int width, int height) {
    if (!main_fbo_) {
        return;
    }
    GLuint fbo = static_cast<GLuint>(*main_fbo_);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
}

void OITRenderer::composite(int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    composite_shader_.start();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, oit_accum_tex_ ? static_cast<GLuint>(*oit_accum_tex_) : 0);
    composite_shader_.set_accum_texture(0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, oit_reveal_tex_ ? static_cast<GLuint>(*oit_reveal_tex_) : 0);
    composite_shader_.set_reveal_texture(1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, main_color_tex_ ? static_cast<GLuint>(*main_color_tex_) : 0);
    composite_shader_.set_opaque_texture(2);
    static GLuint fullscreen_vao = 0;
    if (fullscreen_vao == 0) {
        glGenVertexArrays(1, &fullscreen_vao);
    }
    glBindVertexArray(fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    composite_shader_.stop();
    glEnable(GL_DEPTH_TEST);
}

bool OITRenderer::create_targets(int width, int height) {
    destroy_targets();

    main_fbo_ = std::make_shared<FBOData>();
    oit_fbo_ = std::make_shared<FBOData>();
    if (!main_fbo_ || !oit_fbo_) {
        logging::log(0, logging::ERROR, "OITRenderer: failed to allocate FBO objects");
        return false;
    }

    auto color_spec = make_texture_spec(GL_RGBA16F, GL_RGBA, GL_FLOAT);
    auto depth_spec = make_texture_spec(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
    auto reveal_spec = make_texture_spec(GL_R16F, GL_RED, GL_FLOAT);

    main_color_tex_ = main_fbo_->create_color_attachment(width, height, color_spec, GL_COLOR_ATTACHMENT0);
    depth_tex_ = main_fbo_->create_depth_attachment(width, height, depth_spec);
    if (!main_fbo_->check_status()) {
        logging::log(0, logging::ERROR, "OITRenderer: main framebuffer incomplete");
        return false;
    }

    oit_accum_tex_ = oit_fbo_->create_color_attachment(width, height, color_spec, GL_COLOR_ATTACHMENT0);
    oit_reveal_tex_ = oit_fbo_->create_color_attachment(width, height, reveal_spec, GL_COLOR_ATTACHMENT1);
    if (depth_tex_) {
        oit_fbo_->attach_texture(GL_DEPTH_ATTACHMENT, depth_tex_);
    }
    oit_fbo_->bind();
    GLenum oit_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, oit_buffers);
    oit_fbo_->unbind();
    if (!oit_fbo_->check_status()) {
        logging::log(0, logging::ERROR, "OITRenderer: OIT framebuffer incomplete");
        return false;
    }

    return true;
}

void OITRenderer::destroy_targets() {
    main_color_tex_.reset();
    depth_tex_.reset();
    oit_accum_tex_.reset();
    oit_reveal_tex_.reset();
    main_fbo_.reset();
    oit_fbo_.reset();
}
