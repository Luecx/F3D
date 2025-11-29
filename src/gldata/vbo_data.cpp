#include "vbo_data.h"
#include "../core/glerror.h"

/**
 * @file vbo_data.cpp
 * @brief Implementation of VBOData, a general buffer wrapper for vertex/index data.
 */

VBOData::VBOData(GLenum target) : target(target) {
    glGenBuffers(1, &data_id);
    GL_ERROR_CHECK();
}

VBOData::~VBOData() {
    if (data_id != 0) {
        glDeleteBuffers(1, &data_id);
        data_id = 0;
    }
}

void VBOData::bind() {
    glBindBuffer(target, data_id);
    GL_ERROR_CHECK();
}

void VBOData::unbind() {
    glBindBuffer(target, 0);
    GL_ERROR_CHECK();
}

void VBOData::allocate(GLsizeiptr size, const void* data, GLenum usage) {
    bind();
    glBufferData(target, size, data, usage);
    GL_ERROR_CHECK();
    unbind();
}

void VBOData::update_subdata(GLintptr offset, GLsizeiptr size, const void* data) {
    bind();
    glBufferSubData(target, offset, size, data);
    GL_ERROR_CHECK();
    unbind();
}

void VBOData::store_data(int attributeNumber, int dimensions, std::vector<float>& data) {
    // High-level convenience: upload and configure attribute pointer
    target = GL_ARRAY_BUFFER;
    bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
    GL_ERROR_CHECK();

    glVertexAttribPointer(attributeNumber, dimensions, GL_FLOAT, GL_FALSE,
                          dimensions * static_cast<GLint>(sizeof(float)), nullptr);
    GL_ERROR_CHECK();

    unbind();
}

void VBOData::store_data(int attributeNumber, int dimensions, std::vector<int>& data) {
    // High-level convenience: upload and configure integer attribute pointer
    target = GL_ARRAY_BUFFER;
    bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * data.size(), data.data(), GL_STATIC_DRAW);
    GL_ERROR_CHECK();

    glVertexAttribIPointer(attributeNumber, dimensions, GL_INT, 0, nullptr);
    GL_ERROR_CHECK();

    unbind();
}

void VBOData::store_indices(const std::vector<uint32_t>& indices) {
    // Temporarily treat this as an index buffer.
    GLenum oldTarget = target;
    target = GL_ELEMENT_ARRAY_BUFFER;

    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(uint32_t),
                 indices.data(),
                 GL_STATIC_DRAW);
    GL_ERROR_CHECK();

    unbind();
    target = oldTarget;
}
