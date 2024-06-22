//
// Created by Finn Eggers on 05.06.24.
//

#ifndef F3D_SHADOW_CASTER_H
#define F3D_SHADOW_CASTER_H

#include "../../ecs/include.h"
#include "../../gldata/FBOData.h"
#include "../../math/mat.h"
#include "lights.h"

struct ShadowCaster : ecs::ComponentBaseOf<ShadowCaster> {

    private:
    // holds a value of the fbo whcih will be assigned via the shadow mapping system
    FBODataUPtr shadow_fbo;

    // holds a resolution value for the shadow map
    int resolution = 1024;

    // stores a type for the shadow map
    TextureType type = TextureType::TEX_2D;

    bool active = true;

    public:

    // constructions
    ShadowCaster(int resolution = 1024) : resolution(resolution) {}

    // assign the fbo to the shadow caster
    void set_fbo(FBODataUPtr fbo) {
        shadow_fbo = std::move(fbo);
    }
    void set_resolution(int res) {
        resolution = res;
    }

    // get fbo pointer
    FBOData* get_fbo() {
            return shadow_fbo.get();
    }

    virtual void component_removed() override {
        // delete fbo
        shadow_fbo = nullptr;
    }
    virtual void entity_activated() override {
        // create fbo
        shadow_fbo = std::make_unique<FBOData>(type);
        //shadow_fbo->create_depth_attachment(resolution, resolution);
    }
    virtual void entity_deactivated() override {
        // delete fbo
        shadow_fbo = nullptr;
    }
    virtual void other_component_added(ecs::Hash hash) override {
        // if the light component is one of the three lights, the shadow caster will be activated
        if (   hash == ecs::get_type_hash<DirectionalLight>()
            || hash == ecs::get_type_hash<PointLight>()
            || hash == ecs::get_type_hash<SpotLight>()) {
            // store type and set active
            type = TextureType::TEX_2D;
            active = true;
        }
    }
    virtual void other_component_removed(ecs::Hash hash) override {
        // if the light component is one of the three lights, the shadow caster will be deactivated
        if (   hash == ecs::get_type_hash<DirectionalLight>()
            || hash == ecs::get_type_hash<PointLight>()
            || hash == ecs::get_type_hash<SpotLight>()) {
            active = false;
        }
    }
};

#endif    // F3D_SHADOW_CASTER_H
