//
// Created by finn on 5/22/24.
//

#ifndef ENGINE3D_TEXTUREID_H
#define ENGINE3D_TEXTUREID_H

#include "GLData.h"

#include <memory>

enum class TextureType{
    TEX_2D = GL_TEXTURE_2D,
    TEX_CUBE_MAP = GL_TEXTURE_CUBE_MAP
};

class TextureData : public GLData{

    private:
    TextureType type;

    public:
    TextureData(TextureType type);
    ~TextureData();

    public:
    void bind() override;
    void unbind() override;
    void set_data(int width, int height, GLenum format, GLenum type, const void* data[6]);
};

using TextureDataPtr = std::shared_ptr<TextureData>;

#endif    // ENGINE3D_TEXTUREID_H
