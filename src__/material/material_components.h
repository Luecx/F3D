#pragma once

#include "../resources/texture_resource.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

// Enum to select between constant and texture mode.
enum class ComponentMode { CONSTANT, TEXTURE };

struct ColorComponent {
    ComponentMode mode{ComponentMode::CONSTANT};
    float r{1.0f};
    float g{1.0f};
    float b{1.0f};
    std::shared_ptr<TextureResource> texture;

    void set_constant(float r_val, float g_val, float b_val) {
        r = r_val;
        g = g_val;
        b = b_val;
        texture.reset();
        mode = ComponentMode::CONSTANT;
    }

    void set_texture(const std::shared_ptr<TextureResource>& tex) {
        texture = tex;
        mode = tex ? ComponentMode::TEXTURE : ComponentMode::CONSTANT;
    }

    void print(const std::string& name) const {
        if (mode == ComponentMode::TEXTURE && texture) {
            std::cout << std::setw(25) << name << ": texture = " << texture->get_path() << "\n";
        } else {
            std::cout << std::setw(25) << name << ": value   = (" << r << ", " << g << ", " << b << ")\n";
        }
    }
};

struct FloatComponent {
    ComponentMode mode{ComponentMode::CONSTANT};
    float value{0.0f};
    std::shared_ptr<TextureResource> texture;

    void set_constant(float val) {
        value = val;
        texture.reset();
        mode = ComponentMode::CONSTANT;
    }

    void set_texture(const std::shared_ptr<TextureResource>& tex) {
        texture = tex;
        mode = tex ? ComponentMode::TEXTURE : ComponentMode::CONSTANT;
    }

    void print(const std::string& name) const {
        if (mode == ComponentMode::TEXTURE && texture) {
            std::cout << std::setw(25) << name << ": texture = " << texture->get_path() << "\n";
        } else {
            std::cout << std::setw(25) << name << ": value   = " << value << "\n";
        }
    }
};

inline void print_texture_slot(const std::string& name, const std::shared_ptr<TextureResource>& tex) {
    if (tex) {
        std::cout << std::setw(25) << name << ": texture = " << tex->get_path() << "\n";
    } else {
        std::cout << std::setw(25) << name << ": texture = [nullptr]\n";
    }
}
