#pragma once
//
// Framebuffer Object wrapper.
//

#include "gl_data.h"
#include "texture_data.h"
#include <memory>
#include <vector>

/**
 * @brief RAII wrapper for an OpenGL Framebuffer Object (FBO).
 *
 * Supports:
 * - Creation/deletion of a GL framebuffer.
 * - Attaching 2D and cubemap textures as color or depth attachments.
 * - Optional ownership of attachments via shared_ptr<TextureData>.
 * - Status checking via glCheckFramebufferStatus.
 */
class FBOData : public GLData {
  private:
    /// All textures attached to this FBO (color + depth).
    std::vector<TextureData::SPtr> attachments;
    /// Optional dedicated depth attachment.
    TextureData::SPtr depth_attachment;

    /// FBO texture type hint (2D vs cube map).
    TextureType type;

  public:
    /**
     * @brief Create a new FBO wrapper.
     *
     * @param type Texture type used by color/depth attachments (2D or cube map).
     */
    explicit FBOData(TextureType type = TextureType::TEX_2D);

    /// Destructor: deletes the underlying framebuffer.
    ~FBOData() override;

    /**
     * @brief Bind this framebuffer as GL_FRAMEBUFFER.
     */
    void bind() override;

    /**
     * @brief Unbind the framebuffer (binds 0 to GL_FRAMEBUFFER).
     */
    void unbind() override;

    /**
     * @brief Create and attach a depth texture to this FBO.
     *
     * The texture will be allocated according to the given specification and
     * attached as GL_DEPTH_ATTACHMENT (or depth-stencil equivalent).
     *
     * @param width         Texture width.
     * @param height        Texture height.
     * @param specification Texture specification.
     * @return Shared pointer to the created depth texture.
     */
    TextureData::SPtr create_depth_attachment(int width,
                                              int height,
                                              const TextureSpecification& specification);

    /**
     * @brief Create and attach a color texture to this FBO.
     *
     * The texture will be allocated according to the given specification and
     * attached to the provided attachment point (e.g., GL_COLOR_ATTACHMENT0).
     *
     * @param width         Texture width.
     * @param height        Texture height.
     * @param specification Texture specification.
     * @param attachment    Color attachment slot (e.g., GL_COLOR_ATTACHMENT0).
     * @return Shared pointer to the created color texture.
     */
    TextureData::SPtr create_color_attachment(int width,
                                              int height,
                                              const TextureSpecification& specification,
                                              GLenum attachment = GL_COLOR_ATTACHMENT0);

    /**
     * @brief Attach an existing texture to this FBO.
     *
     * Supports 2D and cube-map textures. For cube maps, all six faces are
     * attached to the same FBO attachment point.
     *
     * @param attachment FBO attachment enum (GL_COLOR_ATTACHMENTi, GL_DEPTH_ATTACHMENT, etc.).
     * @param texture    Shared pointer to an existing TextureData. If null, this is a no-op.
     */
    void attach_texture(GLenum attachment, const TextureData::SPtr& texture);

    /**
     * @brief Get the depth texture attached to this FBO, if any.
     *
     * @return Pointer to the depth TextureData, or nullptr if none is attached.
     */
    [[nodiscard]] TextureData* depth_texture() const { return depth_attachment.get(); }

    /**
     * @brief Check whether this FBO is complete.
     *
     * Binds the FBO, calls glCheckFramebufferStatus(GL_FRAMEBUFFER) and unbinds.
     *
     * @return true if glCheckFramebufferStatus returns GL_FRAMEBUFFER_COMPLETE.
     */
    [[nodiscard]] bool check_status();
};

