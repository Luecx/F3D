#pragma once
//
// Texture wrapper supporting 2D and cube-map textures with bindless handles.
//

#include "gl_data.h"
#include <memory>

/**
 * @brief Enum describing supported texture object types.
 */
enum class TextureType {
    TEX_2D      = GL_TEXTURE_2D,
    TEX_CUBE_MAP = GL_TEXTURE_CUBE_MAP
};

/**
 * @brief Description of texture storage, format, and sampling parameters.
 *
 * This structure specifies how the texture is allocated and sampled:
 * - Internal format (e.g., GL_RGBA8, GL_SRGB8_ALPHA8)
 * - Data format and type for upload
 * - Min/mag filters and wrap modes
 * - Whether mipmaps should be generated automatically.
 */
struct TextureSpecification {
    TextureType type = TextureType::TEX_2D;

    GLint  internal_format = GL_RGBA8;
    GLenum data_format     = GL_RGBA;
    GLenum data_type       = GL_UNSIGNED_BYTE;

    GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;
    GLint mag_filter = GL_LINEAR;

    GLint wrap_s = GL_REPEAT;
    GLint wrap_t = GL_REPEAT;
    GLint wrap_r = GL_REPEAT;

    bool generate_mipmaps = true;
};

/**
 * @brief RAII wrapper around an OpenGL texture object, with optional bindless handle.
 *
 * This class supports:
 * - 2D textures and cube maps.
 * - Allocation via set_data(), optionally providing initial pixel data.
 * - Querying a 64-bit bindless handle where supported (GL_ARB_bindless_texture).
 */
class TextureData : public GLData {
  private:
    TextureSpecification spec{};
    GLuint64 bindless_handle{0};
    int width{0};
    int height{0};

    /// Ensure that a GL texture object exists.
    void ensure_created();

  public:
    using SPtr = std::shared_ptr<TextureData>;
    using UPtr = std::unique_ptr<TextureData>;

    /**
     * @brief Create a texture wrapper with a given type (2D or cube map).
     *
     * The underlying GL texture is not created until set_data() or bind() is called.
     *
     * @param type Texture type (2D or cube map).
     */
    explicit TextureData(TextureType type);

    /**
     * @brief Destructor: deletes the GL texture and (if applicable) the bindless handle.
     */
    ~TextureData() override;

    /**
     * @brief Bind this texture to its appropriate target.
     *
     * For 2D textures, this binds GL_TEXTURE_2D.
     * For cube maps, this binds GL_TEXTURE_CUBE_MAP.
     */
    void bind() override;

    /**
     * @brief Unbind this texture from its appropriate target.
     */
    void unbind() override;

    /**
     * @brief Allocate storage and upload data for this texture.
     *
     * For 2D textures, only @c data[0] is used; for cube maps, all six entries
     * are used in order POSITIVE_X, NEGATIVE_X, POSITIVE_Y, NEGATIVE_Y,
     * POSITIVE_Z, NEGATIVE_Z.
     *
     * @param w             Width in pixels.
     * @param h             Height in pixels.
     * @param specification Texture specification (format, filter, wrap, etc.).
     * @param data          Array of up to 6 pointers to pixel data. May be nullptr
     *                      or contain nullptr entries to allocate without upload.
     */
    void set_data(int w, int h, const TextureSpecification& specification, const void* const data[6]);

    /**
     * @brief Get the 64-bit texture handle.
     *
     * If bindless textures are supported, this returns the bindless handle made
     * resident with glMakeTextureHandleResidentARB. Otherwise, it returns the
     * 32-bit texture id cast to 64 bits.
     *
     * @return 64-bit texture handle usable in shaders (if bindless), otherwise id.
     */
    [[nodiscard]] GLuint64 get_handle() const;

    /// Returns the texture type.
    [[nodiscard]] TextureType get_type() const { return spec.type; }

    /// Returns the texture width in pixels.
    [[nodiscard]] int get_width() const { return width; }

    /// Returns the texture height in pixels.
    [[nodiscard]] int get_height() const { return height; }

    /// Returns the specification used to allocate this texture.
    [[nodiscard]] const TextureSpecification& get_spec() const { return spec; }
};

