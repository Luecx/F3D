#pragma once
//
// General Shader Storage Buffer Object (SSBO) wrapper.
//
// Provides:
// - RAII GL buffer handle.
// - Explicit allocate() and update_subdata() APIs.
// - Convenience full update via update_data().
// - Binding to a shader storage binding point.
//
// This is a *general* SSBO; higher-level code decides the layout.
//

#include "gl_data.h"

/**
 * @brief RAII wrapper for an OpenGL Shader Storage Buffer Object (SSBO).
 *
 * This class owns a GL buffer whose primary usage target is
 * GL_SHADER_STORAGE_BUFFER. It provides:
 * - Lazy or explicit creation of the buffer object.
 * - Allocation with glBufferData.
 * - Sub-region updates via glBufferSubData.
 * - Binding to a specified binding point for use with `layout(std430, binding = X)` blocks.
 */
class SSBOData : public GLData {
  public:
    /**
     * @brief Construct an empty SSBO wrapper.
     *
     * The underlying GL buffer is not created until either allocate(),
     * update_data(), update_subdata(), or bind() is called.
     */
    SSBOData();

    /**
     * @brief Destructor: deletes the underlying GL buffer if it exists.
     */
    ~SSBOData() override;

    /**
     * @brief Ensure that a GL buffer object exists.
     *
     * If @c data_id is 0, glGenBuffers will be called. Otherwise this is a no-op.
     */
    void ensure_created();

    /**
     * @brief Allocate or reallocate storage for the entire SSBO and upload data.
     *
     * This calls glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage).
     * Existing contents are discarded.
     *
     * @param size  Size in bytes of the new buffer data.
     * @param data  Pointer to initial data (may be nullptr to allocate only).
     * @param usage Usage hint (e.g., GL_DYNAMIC_DRAW, GL_STATIC_DRAW).
     */
    void update_data(GLsizeiptr size, const void* data, GLenum usage = GL_DYNAMIC_DRAW);

    /**
     * @brief Update a sub-region of the existing SSBO.
     *
     * The buffer must already be allocated with sufficient size.
     * This calls glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data).
     *
     * @param size   Size in bytes to update.
     * @param data   Pointer to the data to upload.
     * @param offset Byte offset into the buffer where the update begins.
     * @param usage  Usage hint (kept for compatibility; not used for sub-data).
     */
    void update_data(GLsizeiptr size, const void* data, GLintptr offset, GLenum usage = GL_DYNAMIC_DRAW);

    /**
     * @brief Bind this SSBO to a specific shader storage binding point.
     *
     * This calls glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id()).
     *
     * @param bindingPoint The index used in your shader `layout(binding = X)` declaration.
     */
    void bind(GLuint bindingPoint);

    /**
     * @brief Bind this buffer to GL_SHADER_STORAGE_BUFFER target.
     *
     * This is a lower-level bind used internally by update operations.
     */
    void bind() override;

    /**
     * @brief Unbind the GL_SHADER_STORAGE_BUFFER target.
     */
    void unbind() override;
};

