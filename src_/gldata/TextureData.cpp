//
// Created by finn on 5/22/24.
//

#include "../core/glerror.h"
#include "TextureData.h"

void TextureData::bind() {
    glBindTexture((GLenum) type, data_id);
    GL_ERROR_CHECK();
}
void TextureData::unbind() {
    glBindTexture((GLenum) type, 0);
    GL_ERROR_CHECK();
}
TextureData::TextureData(TextureType type) : type(type) {
    glGenTextures(1, &data_id);
    GL_ERROR_CHECK();
}
TextureData::~TextureData() {
    if (data_id != 0)
        glDeleteTextures(1, &data_id);
    GL_ERROR_CHECK();
}

void TextureData::set_data(int width, int height, GLenum format, GLenum type, const void** data) {
    bind();

    if (this->type == TextureType::TEX_2D) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data[0]);
    } else if (this->type == TextureType::TEX_CUBE_MAP) {
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, type, data[i]);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    unbind();
    GL_ERROR_CHECK();
}
