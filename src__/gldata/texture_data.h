//
// Created by finn on 5/22/24.
//

#ifndef ENGINE3D_TEXTUREID_H
#define ENGINE3D_TEXTUREID_H

#include "gl_data.h"

#include <cstdint>
#include <memory>

enum class TextureType { TEX_2D = GL_TEXTURE_2D, TEX_CUBE_MAP = GL_TEXTURE_CUBE_MAP };

struct TextureSpecification {
    TextureType type = TextureType::TEX_2D;
    GLint internal_format = GL_RGBA8;
    GLenum data_format = GL_RGBA;
    GLenum data_type = GL_UNSIGNED_BYTE;
    GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;
    GLint mag_filter = GL_LINEAR;
    GLint wrap_s = GL_REPEAT;
    GLint wrap_t = GL_REPEAT;
    GLint wrap_r = GL_REPEAT;
    bool generate_mipmaps = true;
};

class TextureData : public GLData {

  private:
    TextureSpecification spec;
    GLuint64 bindless_handle = 0;
    int width = 0;
    int height = 0;
    void ensure_created();

  public:
    TextureData(TextureType type);
    ~TextureData();

  public:
    void bind() override;
    void unbind() override;
    void set_data(int width, int height, const TextureSpecification& specification, const void* const data[6]);

    GLuint64 get_handle() const;
    TextureType get_type() const { return spec.type; }

    using SPtr = std::shared_ptr<TextureData>;
    using UPtr = std::unique_ptr<TextureData>;
};

#endif // ENGINE3D_TEXTUREID_H
