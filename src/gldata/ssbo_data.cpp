#include "ssbo_data.h"
#include "../core/glerror.h"

/**
 * @file ssbo_data.cpp
 * @brief Implementation of SSBOData, a RAII wrapper around GL shader storage buffers.
 */

SSBOData::SSBOData() = default;

SSBOData::~SSBOData() {
    if (data_id != 0) {
        glDeleteBuffers(1, &data_id);
        data_id = 0;
    }
}

void SSBOData::ensure_created() {
    if (data_id == 0) {
        glGenBuffers(1, &data_id);
        GL_ERROR_CHECK();
    }
}

void SSBOData::update_data(GLsizeiptr size, const void* data, GLenum usage) {
    ensure_created();
    bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    GL_ERROR_CHECK();
    unbind();
}

void SSBOData::update_data(GLsizeiptr size, const void* data, GLintptr offset, GLenum /*usage*/) {
    ensure_created();
    bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
    GL_ERROR_CHECK();
    unbind();
}

void SSBOData::bind(GLuint bindingPoint) {
    ensure_created();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, data_id);
    GL_ERROR_CHECK();
}

void SSBOData::bind() {
    ensure_created();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_id);
    GL_ERROR_CHECK();
}

void SSBOData::unbind() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL_ERROR_CHECK();
}
