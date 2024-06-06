//
// Created by Finn Eggers on 01.06.24.
//

#ifndef F3D_RENDERABLE_H
#define F3D_RENDERABLE_H

#include "../ecs/include.h"

class Renderable : ecs::ComponentBaseOf<Renderable> {
public:
    virtual void component_removed() override {}
    virtual void entity_activated() override {}
    virtual void entity_deactivated() override {}
};

#endif    // F3D_RENDERABLE_H
