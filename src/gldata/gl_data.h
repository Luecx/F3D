#pragma once
//
// Base class for all OpenGL object wrappers.
//
// Provides:
// - RAII-style lifetime management for a GLuint handle.
// - Non-copyable, optionally movable base.
// - Common `bind()` / `unbind()` interface where it makes sense.
// - `operator GLuint()` to pass the raw handle into GL calls.
//
// Derive from this class for FBOs, buffers, VAOs, textures, etc.
//

#define GLFW_INCLUDE_NONE
#include "../glad.h"

#include <cstdint>

/**
 * @brief Base class for OpenGL object wrappers that own a single GLuint handle.
 *
 * This class is meant to be inherited by concrete GL resource types such as
 * framebuffers, buffers (VBO, SSBO, etc.), textures, and vertex array objects.
 *
 * Responsibilities:
 * - Store and expose the underlying OpenGL handle.
 * - Provide a polymorphic interface for binding/unbinding where appropriate.
 * - Enforce non-copyable semantics (GL objects are unique resources).
 *
 * This class does *not* itself create or delete the GL object; derived classes
 * are responsible for generating and deleting the underlying handle.
 */
class GLData {
  protected:
    /// Raw OpenGL handle (0 means "no object").
    GLuint data_id{0};

    /// Protected default constructor: only derived classes may instantiate.
    GLData() = default;

  public:
    /// Virtual destructor to allow safe deletion via base pointer.
    virtual ~GLData() = default;

    // Non-copyable: GL objects should not be implicitly duplicated.
    GLData(const GLData&) = delete;
    GLData& operator=(const GLData&) = delete;

    // Movable: allows transferring ownership if needed.
    GLData(GLData&& other) noexcept : data_id(other.data_id) { other.data_id = 0; }
    GLData& operator=(GLData&& other) noexcept {
        if (this != &other) {
            data_id = other.data_id;
            other.data_id = 0;
        }
        return *this;
    }

    /**
     * @brief Implicit conversion operator to the underlying GLuint handle.
     *
     * This allows passing GLData-derived objects directly into OpenGL functions
     * that expect a raw GLuint (e.g., glBindTexture, glBindFramebuffer, etc.).
     *
     * @return The underlying OpenGL handle (may be 0 if not created).
     */
    explicit operator GLuint() const { return data_id; }

    /**
     * @brief Returns the underlying OpenGL handle.
     *
     * @return GLuint handle managed by this object.
     */
    GLuint id() const { return data_id; }

    /**
     * @brief Returns whether this object currently owns a valid GL handle.
     *
     * @return true if data_id != 0, false otherwise.
     */
    bool valid() const { return data_id != 0; }

    /**
     * @brief Bind this object to its respective OpenGL target.
     *
     * The specific target (e.g., GL_FRAMEBUFFER, GL_ARRAY_BUFFER, etc.)
     * is defined by the derived class implementation.
     */
    virtual void bind() = 0;

    /**
     * @brief Unbind this object from its respective OpenGL target.
     *
     * The specific target is defined by the derived class implementation.
     * Many unbind operations will bind 0 on the corresponding target.
     */
    virtual void unbind() = 0;
};

