#include "vao_data.h"
#include "../core/glerror.h"

/**
 * @file vao_data.cpp
 * @brief Implementation of VAOData, a wrapper for OpenGL vertex array objects.
 */

VAOData::VAOData() {
    glGenVertexArrays(1, &data_id);
    GL_ERROR_CHECK();
}

VAOData::~VAOData() {
    unbind();
    if (data_id != 0) {
        glDeleteVertexArrays(1, &data_id);
        data_id = 0;
        GL_ERROR_CHECK();
    }
}

void VAOData::bind() {
    glBindVertexArray(data_id);
    GL_ERROR_CHECK();
}

void VAOData::unbind() {
    glBindVertexArray(0);
    GL_ERROR_CHECK();
}

void VAOData::set_attribute(GLuint index,
                            GLint size,
                            GLenum type,
                            GLboolean normalized,
                            GLsizei stride,
                            const void* offset,
                            const VBOData::SPtr& vbo,
                            bool integerAttribute) {
    bind();
    vbo->bind();

    glEnableVertexAttribArray(index);
    GL_ERROR_CHECK();

    if (integerAttribute) {
        glVertexAttribIPointer(index, size, type, stride, offset);
    } else {
        glVertexAttribPointer(index, size, type, normalized, stride, offset);
    }
    GL_ERROR_CHECK();

    // Leave VAO bound or not depending on your convention; here we unbind VBO only.
    vbo->unbind();
}
