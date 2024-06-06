//
// Created by Finn Eggers on 01.06.24.
//

#ifndef F3D_LIGHTS_H
#define F3D_LIGHTS_H

#include "../../ecs/include.h"
#include "../../gldata/FBOData.h"
#include "../../math/mat.h"

#include <memory>
#include <vector>

struct Light {
    public:
    Vec3f color;

    protected:
    // texture type for the shadow map
    TextureType type = TextureType::TEX_2D;

    // construction for color and type
    Light(Vec3f color, TextureType type = TextureType::TEX_2D)
        : color(color)
        , type(type) {}

};

// three types of lights: directional, point and spot, each overwriting the light struct and ecs::ComponentBaseOf<T>
struct DirectionalLight : Light, ecs::ComponentBaseOf<DirectionalLight> {
    DirectionalLight(Vec3f color)
        : Light(color, type) {}

    virtual void component_removed() override {}
    virtual void entity_activated() override {}
    virtual void entity_deactivated() override {}
    virtual void other_component_added(ecs::Hash hash) override {}
    virtual void other_component_removed(ecs::Hash hash) override {}
};

struct PointLight : Light, ecs::ComponentBaseOf<PointLight> {
    float half_radius = 0.0f;

    PointLight(Vec3f color, float half_radius)
        : Light(color, TextureType::TEX_CUBE_MAP)
        , half_radius(half_radius) {}

    virtual void component_removed() override {}
    virtual void entity_activated() override {}
    virtual void entity_deactivated() override {}
    virtual void other_component_added(ecs::Hash hash) override {}
    virtual void other_component_removed(ecs::Hash hash) override {}
};

struct SpotLight : Light, ecs::ComponentBaseOf<SpotLight> {
    float half_radius = 0.0f;
    float angle = 0.0f;

    SpotLight(Vec3f color, float half_radius, float angle)
        : Light(color, TextureType::TEX_2D)
        , half_radius(half_radius)
        , angle(angle) {}

    virtual void component_removed() override {}
    virtual void entity_activated() override {}
    virtual void entity_deactivated() override {}
    virtual void other_component_added(ecs::Hash hash) override {}
    virtual void other_component_removed(ecs::Hash hash) override {}
};

#endif    // F3D_LIGHTS_H
