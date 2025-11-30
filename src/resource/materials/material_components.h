#pragma once

#include <memory>
#include <string>
#include "../../logging/logging.h"
#include "../resource_logging.h"

class Texture;  // forward declaration

/**
 * @brief Mode for a material component.
 *
 * A component can either be a plain constant value or be driven by a texture.
 */
enum class ComponentMode {
    CONSTANT = 0, ///< Use only the stored constant value.
    TEXTURE  = 1  ///< Sample from the referenced texture.
};

/**
 * @brief Color component of a material (RGB), optionally driven by a texture.
 *
 * The texture pointer is kept separate from the constant value. When a texture
 * is enabled, the constant still acts as a default / fallback color.
 */
struct ColorComponent {
    ComponentMode mode{ComponentMode::CONSTANT}; ///< Constant vs. textured.
    float r{1.0f};                               ///< Red   channel.
    float g{1.0f};                               ///< Green channel.
    float b{1.0f};                               ///< Blue  channel.

    std::shared_ptr<Texture> texture;            ///< Optional texture source.

    /**
     * @brief Set this component to a constant color and clear any texture.
     */
    void set_constant(float r_in, float g_in, float b_in) {
        mode    = ComponentMode::CONSTANT;
        r       = r_in;
        g       = g_in;
        b       = b_in;
        texture.reset();
    }

    /**
     * @brief Set this component to be driven by @p tex with a default color.
     *
     * The default color is used both as a fallback and as the value when the
     * texture is not yet available on the GPU.
     */
    void set_texture(const std::shared_ptr<Texture>& tex,
                     float default_r = 1.0f,
                     float default_g = 1.0f,
                     float default_b = 1.0f) {
        mode    = ComponentMode::TEXTURE;
        texture = tex;
        r       = default_r;
        g       = default_g;
        b       = default_b;
    }

    /**
     * @brief Print a human-readable description, mainly for debugging.
     */
    void print(const std::string& label) const {
        logging::log(reslog::MATERIAL, logging::DEBUG,
                     "  " + label + " = ColorComponent(mode=" +
                         std::string(mode == ComponentMode::CONSTANT ? "CONSTANT" : "TEXTURE") +
                         ", value=(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")" +
                         ", has_texture=" + (texture ? std::string("yes") : std::string("no")) + ")");
    }
};

/**
 * @brief Scalar component of a material, optionally driven by a texture.
 */
struct FloatComponent {
    ComponentMode mode{ComponentMode::CONSTANT}; ///< Constant vs. textured.
    float value{0.0f};                           ///< Scalar value.
    std::shared_ptr<Texture> texture;            ///< Optional texture source.

    /**
     * @brief Set this component to a constant scalar value and clear any texture.
     */
    void set_constant(float v) {
        mode    = ComponentMode::CONSTANT;
        value   = v;
        texture.reset();
    }

    /**
     * @brief Set this component to be driven by @p tex with a default value.
     *
     * The default value is used both as a fallback and as the value when the
     * texture is not yet available on the GPU.
     */
    void set_texture(const std::shared_ptr<Texture>& tex, float default_value = 0.0f) {
        mode    = ComponentMode::TEXTURE;
        texture = tex;
        value   = default_value;
    }

    /**
     * @brief Print a human-readable description, mainly for debugging.
     */
    void print(const std::string& label) const {
        logging::log(reslog::MATERIAL, logging::DEBUG,
                     "  " + label + " = FloatComponent(mode=" +
                         std::string(mode == ComponentMode::CONSTANT ? "CONSTANT" : "TEXTURE") +
                         ", value=" + std::to_string(value) +
                         ", has_texture=" + (texture ? std::string("yes") : std::string("no")) + ")");
    }
};

/**
 * @brief Utility to print a texture-only slot (for debugging).
 */
inline void print_texture_slot(const std::string& label,
                               const std::shared_ptr<Texture>& tex) {
    logging::log(reslog::MATERIAL, logging::DEBUG,
                 "  " + label + " = TextureSlot(has_texture=" + (tex ? std::string("yes") : std::string("no")) + ")");
}
