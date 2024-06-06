//
// Created by Finn Eggers on 01.06.24.
//

#ifndef F3D_MODEL_H
#define F3D_MODEL_H

#include "../resources/geometry.h"
#include "../ecs/include.h"

struct Model : public ecs::ComponentBaseOf<Model>{
    // pointer to a resource
    std::shared_ptr<GeometryResource> resource;

    // construct with a resource
    Model(std::shared_ptr<GeometryResource> resource) : resource(resource) {}

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
};


#endif    // F3D_MODEL_H
