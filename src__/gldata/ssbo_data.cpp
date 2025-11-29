#include "ssbo_data.h"

// Constructor: Generate the buffer.
SSBOData::SSBOData() = default;

// Destructor: Delete the buffer.
SSBOData::~SSBOData() {
    if (data_id != 0) {
        glDeleteBuffers(1, &data_id);
    }
}

void SSBOData::ensure_created() {
    if (data_id == 0) {
        glGenBuffers(1, &data_id);
    }
}

// Update the entire buffer's data.
// This binds the buffer as a GL_SHADER_STORAGE_BUFFER, uploads new data,
// then unbinds the buffer.
void SSBOData::update_data(GLsizeiptr size, const void* data, GLenum usage) {
    ensure_created();
    bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    unbind();
}

// Update a sub-region of the buffer's data.
void SSBOData::update_data(GLsizeiptr size, const void* data, GLintptr offset, GLenum usage) {
    ensure_created();
    bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
    unbind();
}

// Bind this SSBO to a specified shader binding point.
void SSBOData::bind(GLuint bindingPoint) {
    ensure_created();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, data_id);
}

// Bind the SSBO as the current buffer (without specifying a binding point).
void SSBOData::bind() {
    ensure_created();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_id);
}

// Unbind the SSBO from the target.
void SSBOData::unbind() { glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); }
