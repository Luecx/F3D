#pragma once

#include <memory>

#include "../textures/texture.h"

enum class ComponentMode : unsigned char {
    CONSTANT = 0,
    TEXTURE  = 1
};

struct ColorComponent {
    ComponentMode mode{ComponentMode::CONSTANT};
    float r{1.0f}, g{1.0f}, b{1.0f};
    std::shared_ptr<Texture> texture;

    void set_constant(float r_in, float g_in, float b_in) {
        r = r_in;
        g = g_in;
        b = b_in;
        texture.reset();
        mode = ComponentMode::CONSTANT;
    }

    void set_texture(const std::shared_ptr<Texture>& tex) {
        texture = tex;
        mode = tex ? ComponentMode::TEXTURE : ComponentMode::CONSTANT;
    }
};

struct FloatComponent {
    ComponentMode mode{ComponentMode::CONSTANT};
    float value{0.0f};
    std::shared_ptr<Texture> texture;

    void set_constant(float v) {
        value = v;
        texture.reset();
        mode = ComponentMode::CONSTANT;
    }

    void set_texture(const std::shared_ptr<Texture>& tex) {
        texture = tex;
        mode = tex ? ComponentMode::TEXTURE : ComponentMode::CONSTANT;
    }
};
