//
// Created by finn on 5/22/24.
//

#include "../core/glerror.h"
#include "texture_data.h"

void TextureData::bind() {
    ensure_created();
    glBindTexture(static_cast<GLenum>(spec.type), data_id);
    GL_ERROR_CHECK();
}

void TextureData::unbind() {
    glBindTexture(static_cast<GLenum>(spec.type), 0);
    GL_ERROR_CHECK();
}

TextureData::TextureData(TextureType type) { spec.type = type; }

void TextureData::ensure_created() {
    if (data_id == 0) {
        glGenTextures(1, &data_id);
        GL_ERROR_CHECK();
    }
}

TextureData::~TextureData() {
    if (data_id != 0) {
        glDeleteTextures(1, &data_id);
    }
    GL_ERROR_CHECK();
}

void TextureData::set_data(int w, int h, const TextureSpecification& specification, const void* const data[6]) {
    spec = specification;
    width = w;
    height = h;

    ensure_created();
    bind();
    GLenum target = static_cast<GLenum>(spec.type);

    if (spec.type == TextureType::TEX_2D) {
        glTexImage2D(target, 0, spec.internal_format, width, height, 0, spec.data_format, spec.data_type,
                     data ? data[0] : nullptr);
    } else if (spec.type == TextureType::TEX_CUBE_MAP) {
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, spec.internal_format, width, height, 0,
                         spec.data_format, spec.data_type, data ? data[i] : nullptr);
        }
    }

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, spec.min_filter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, spec.mag_filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, spec.wrap_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, spec.wrap_t);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, spec.wrap_r);

    if (spec.generate_mipmaps) {
        glGenerateMipmap(target);
    }

    unbind();
    GL_ERROR_CHECK();

#ifdef GL_ARB_bindless_texture
    if (glGetTextureHandleARB) {
        bindless_handle = glGetTextureHandleARB(data_id);
        if (glMakeTextureHandleResidentARB) {
            glMakeTextureHandleResidentARB(bindless_handle);
        }
    } else {
        bindless_handle = static_cast<GLuint64>(data_id);
    }
#else
    bindless_handle = static_cast<GLuint64>(data_id);
#endif
}

GLuint64 TextureData::get_handle() const { return bindless_handle ? bindless_handle : static_cast<GLuint64>(data_id); }
