#include "FBOData.h"
#include "../core/glerror.h"

FBOData::FBOData() {
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
    auto attach = std::make_shared<TextureData>(TextureType::TEX_2D);
    const void* data[1] = {nullptr};
    attach->set_data(width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);

    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attach->operator GLuint(), 0);
    GL_ERROR_CHECK();
    check_status();
    unbind();

    attachments.push_back(attach);
}

void FBOData::create_color_attachment(int width, int height, int attachmentIndex) {
    auto attach = std::make_shared<TextureData>(TextureType::TEX_2D);
    const void* data[1] = {nullptr};
    attach->set_data(width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_TEXTURE_2D, attach->operator GLuint(), 0);
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
