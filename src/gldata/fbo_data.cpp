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

void FBOData::create_depth_attachment(int width, int height) {
    auto attach = std::make_shared<TextureData>(type);
    const void* data[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    attach->set_data(width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);

    bind();
    if (type == TextureType::TEX_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attach->operator GLuint(), 0);
    } else if (type == TextureType::TEX_CUBE_MAP) {
        for (unsigned int i = 0; i < 6; ++i) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, attach->operator GLuint(), 0);
        }
    }
    GL_ERROR_CHECK();
    check_status();
    unbind();

    attachments.push_back(attach);
}

void FBOData::create_color_attachment(int width, int height) {
    auto attach = std::make_shared<TextureData>(type);
    const void* data[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    attach->set_data(width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    bind();
    if (type == TextureType::TEX_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attach->operator GLuint(), 0);
    } else if (type == TextureType::TEX_CUBE_MAP) {
        for (unsigned int i = 0; i < 6; ++i) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, attach->operator GLuint(), 0);
        }
    }
    GL_ERROR_CHECK();
    check_status();
    unbind();

    attachments.push_back(attach);
}

bool FBOData::check_status() {
    bind();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    unbind();
    return status == GL_FRAMEBUFFER_COMPLETE;
}
