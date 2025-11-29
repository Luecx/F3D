#pragma once
//
// General buffer wrapper primarily used for vertex/index data.
//

#include "gl_data.h"
#include <memory>
#include <vector>

/**
 * @brief RAII wrapper for an OpenGL buffer object (primarily VBO/EBO).
 *
 * By default this represents a vertex buffer (GL_ARRAY_BUFFER), but the target
 * can be changed to GL_ELEMENT_ARRAY_BUFFER or others as needed.
 *
 * Provides:
 * - Creation/destruction of the GL buffer.
 * - Binding/unbinding to a specified target.
 * - General allocation and sub-data update functions.
 * - Backwards-compatible helper functions for simple attribute/index uploads.
 */
class VBOData : public GLData {
  private:
    GLenum target{GL_ARRAY_BUFFER};

  public:
    using SPtr = std::shared_ptr<VBOData>;
    using UPtr = std::unique_ptr<VBOData>;

    /**
     * @brief Construct a new buffer object with an optional target.
     *
     * @param target OpenGL buffer target (default GL_ARRAY_BUFFER).
     */
    explicit VBOData(GLenum target = GL_ARRAY_BUFFER);

    /**
     * @brief Destructor: deletes the GL buffer object.
     */
    ~VBOData() override;

    /**
     * @brief Bind this buffer to its configured target.
     */
    void bind() override;

    /**
     * @brief Unbind this buffer from its configured target.
     */
    void unbind() override;

    /**
     * @brief Get the currently configured target of this buffer.
     */
    [[nodiscard]] GLenum get_target() const { return target; }

    /**
     * @brief Change the target for this buffer wrapper.
     *
     * Note: This does not move existing GL state; it only affects subsequent
     * bind/unbind calls. Use with care.
     */
    void set_target(GLenum newTarget) { target = newTarget; }

    /**
     * @brief Allocate or reallocate storage for the entire buffer.
     *
     * Equivalent to glBufferData(target, size, data, usage) after binding.
     *
     * @param size  Size in bytes to allocate.
     * @param data  Pointer to initial data (may be nullptr).
     * @param usage Usage hint (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc.).
     */
    void allocate(GLsizeiptr size, const void* data, GLenum usage);

    /**
     * @brief Update a sub-region of the buffer.
     *
     * Calls glBufferSubData(target, offset, size, data) after binding.
     *
     * @param offset Byte offset into the buffer to start writing.
     * @param size   Number of bytes to write.
     * @param data   Pointer to the data to upload.
     */
    void update_subdata(GLintptr offset, GLsizeiptr size, const void* data);

    /**
     * @brief Convenience function to upload float attribute data and set a vertex attribute pointer.
     *
     * This is a high-level helper intended for simple cases. For more control,
     * use allocate(), update_subdata() and VAOData::set_attribute() directly.
     *
     * @param attributeNumber Index of the vertex attribute in the shader.
     * @param dimensions      Number of components per vertex (1–4).
     * @param data            Vector of float data.
     */
    void store_data(int attributeNumber, int dimensions, std::vector<float>& data);

    /**
     * @brief Convenience function to upload integer attribute data and set a vertex attribute pointer.
     *
     * @param attributeNumber Index of the vertex attribute in the shader.
     * @param dimensions      Number of components per vertex (1–4).
     * @param data            Vector of integer data.
     */
    void store_data(int attributeNumber, int dimensions, std::vector<int>& data);

    /**
     * @brief Convenience function to upload index data to GL_ELEMENT_ARRAY_BUFFER.
     *
     * This temporarily binds the buffer as GL_ELEMENT_ARRAY_BUFFER and uploads
     * the given indices as GL_STATIC_DRAW.
     *
     * @param indices Index data to upload.
     */
    void store_indices(const std::vector<uint32_t>& indices);
};

