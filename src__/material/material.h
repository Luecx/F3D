#pragma once

#include "material_properties.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

class Material {
  public:
    using SPtr = std::shared_ptr<Material>;
    using UPtr = std::unique_ptr<Material>;

    Material();
    explicit Material(std::string name);

    void set_default_material();
    MaterialProperties& properties();
    const MaterialProperties& properties() const;

    void set_float_property(const std::string& name, float value,
                            const std::shared_ptr<TextureResource>& texture = nullptr);

    void set_color_property(const std::string& name, float r, float g, float b,
                            const std::shared_ptr<TextureResource>& texture = nullptr);

    void assign_texture_slot(const std::string& name, const std::shared_ptr<TextureResource>& texture);

    void print_overview() const;

  private:
    FloatComponent* float_component(const std::string& name);
    ColorComponent* color_component(const std::string& name);

    std::string name_;
    MaterialProperties properties_;
};
