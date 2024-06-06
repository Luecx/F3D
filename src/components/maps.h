//
// Created by Finn Eggers on 01.06.24.
//

#ifndef F3D_MAPS_H
#define F3D_MAPS_H

#include "../resources/image.h"
#include "../ecs/include.h"

struct Map : public ecs::ComponentBaseOf<Map>{
    // pointer to a resource
    std::shared_ptr<ImageResource> resource;

    // construct with a resource
    Map(std::shared_ptr<ImageResource> resource) : resource(resource) {}

    virtual void component_removed() {
        resource->unrequire();
        resource = nullptr;
    }
    virtual void entity_activated() {
        resource->require();
    }
    virtual void entity_deactivated() {
        resource->unrequire();
    }
    virtual ecs::Hash hash() const {
        return ecs::get_type_hash<ecs::ComponentInterface>();
    }
};


// different types of maps, ColorMap, HeightMap, NormalMap,

struct DiffuseMap : public Map {
    DiffuseMap(std::shared_ptr<ImageResource> resource) : Map(resource) {}
};

struct NormalMap : public Map {
    NormalMap(std::shared_ptr<ImageResource> resource) : Map(resource) {}
};

struct HeightMap : public Map {
    HeightMap(std::shared_ptr<ImageResource> resource) : Map(resource) {}
};

struct SpecularMap : public Map {
    SpecularMap(std::shared_ptr<ImageResource> resource) : Map(resource) {}
};

struct GlossMap : public Map {
    GlossMap(std::shared_ptr<ImageResource> resource) : Map(resource) {}
};

struct AmbientMap : public Map {
    AmbientMap(std::shared_ptr<ImageResource> resource) : Map(resource) {}
};

struct EmissiveMap : public Map {
    EmissiveMap(std::shared_ptr<ImageResource> resource) : Map(resource) {}
};

#endif    // F3D_MAPS_H
