#pragma once
//
// Vertex Array Object (VAO) wrapper.
//

#include "gl_data.h"
#include "vbo_data.h"

#include <memory>
#include <vector>

/**
 * @brief RAII wrapper for an OpenGL Vertex Array Object (VAO).
 *
 * A VAO encapsulates:
 * - The enabled vertex attribute arrays.
 * - The vertex buffer bindings for each attribute index.
 * - The index buffer binding (GL_ELEMENT_ARRAY_BUFFER).
 *
 * This class provides:
 * - Creation/destruction of the VAO.
 * - Simple bind/unbind operations.
 * - A minimal association with VBOData objects for bookkeeping.
 */
class VAOData : public GLData {
  private:
    /// Optional bookkeeping: VBOs associated with this VAO (not required for GL).
    std::vector<VBOData::SPtr> vbos;

  public:
    using SPtr = std::shared_ptr<VAOData>;
    using UPtr = std::unique_ptr<VAOData>;

    /**
     * @brief Construct and generate a new VAO.
     */
    VAOData();

    /**
     * @brief Destructor: deletes the VAO.
     */
    ~VAOData() override;

    /**
     * @brief Bind this VAO with glBindVertexArray.
     */
    void bind() override;

    /**
     * @brief Unbind the VAO (binds 0).
     */
    void unbind() override;

    /**
     * @brief Register a VBO as "owned" or "used" by this VAO.
     *
     * This does not perform any GL calls; it is purely for higher-level
     * bookkeeping if you want to keep track of which VBOs are associated
     * with a given VAO.
     *
     * @param vbo Shared pointer to a VBOData.
     */
    void add_vbo(const VBOData::SPtr& vbo) { vbos.push_back(vbo); }

    /**
     * @brief Enable and describe a vertex attribute array in this VAO.
     *
     * This is a convenience function that:
     * - Binds the VAO.
     * - Binds the given VBO to GL_ARRAY_BUFFER.
     * - Enables the attribute index.
     * - Calls glVertexAttribPointer / glVertexAttribIPointer depending on @p integerAttribute.
     *
     * @param index            Attribute index (location in the shader).
     * @param size             Number of components (1–4).
     * @param type             OpenGL type (e.g., GL_FLOAT, GL_INT).
     * @param normalized       Whether fixed-point data is normalized.
     * @param stride           Byte stride between consecutive vertices.
     * @param offset           Byte offset of the first component within the vertex.
     * @param vbo              VBO containing the vertex data.
     * @param integerAttribute If true, uses glVertexAttribIPointer, else glVertexAttribPointer.
     */
    void set_attribute(GLuint index,
                       GLint size,
                       GLenum type,
                       GLboolean normalized,
                       GLsizei stride,
                       const void* offset,
                       const VBOData::SPtr& vbo,
                       bool integerAttribute = false);
};

