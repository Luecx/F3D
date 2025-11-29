#pragma once

#include "gl_data.h"

struct SSBOData : public GLData {

    SSBOData();
    ~SSBOData();

    void ensure_created();

    // Update the buffer's data.
    void update_data(GLsizeiptr size, const void* data, GLenum usage = GL_DYNAMIC_DRAW);
    void update_data(GLsizeiptr size, const void* data, GLsizeiptr offset, GLenum usage = GL_DYNAMIC_DRAW);

    // Bind this SSBO to a shader binding point.
    void bind(GLuint bindingPoint);

    void bind();
    void unbind();
};
