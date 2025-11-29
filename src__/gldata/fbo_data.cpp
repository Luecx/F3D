#include "fbo_data.h"
#include "../core/glerror.h"

FBOData::FBOData(TextureType type) : type(type) {
    glGenFramebuffers(1, &data_id);
    GL_ERROR_CHECK();
    check_status();
}

FBOData::~FBOData() {
    if (data_id != 0) {
        glDeleteFramebuffers(1, &data_id);
    }
    GL_ERROR_CHECK();
    check_status();
}

void FBOData::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, data_id);
    GL_ERROR_CHECK();
}

void FBOData::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_ERROR_CHECK();
}

TextureData::SPtr FBOData::create_depth_attachment(int width, int height, const TextureSpecification& specification) {
    auto attach = std::make_shared<TextureData>(specification.type);
    const void* data[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    attach->set_data(width, height, specification, data);
    attach_texture(GL_DEPTH_ATTACHMENT, attach);
    depth_attachment = attach;
    return attach;
}

TextureData::SPtr FBOData::create_color_attachment(int width,
                                                   int height,
                                                   const TextureSpecification& specification,
                                                   GLenum attachment) {
    auto attach = std::make_shared<TextureData>(specification.type);
    const void* data[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    attach->set_data(width, height, specification, data);
    attach_texture(attachment, attach);
    return attach;
}

void FBOData::attach_texture(GLenum attachment, const TextureData::SPtr& texture) {
    if (!texture) {
        return;
    }

    bind();
    if (texture->get_type() == TextureType::TEX_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->operator GLuint(), 0);
    } else if (texture->get_type() == TextureType::TEX_CUBE_MAP) {
        for (unsigned int i = 0; i < 6; ++i) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   texture->operator GLuint(), 0);
        }
    }
    GL_ERROR_CHECK();
    check_status();
    unbind();

    attachments.push_back(texture);
    if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        depth_attachment = texture;
    }
}

bool FBOData::check_status() {
    bind();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    unbind();
    return status == GL_FRAMEBUFFER_COMPLETE;
}
