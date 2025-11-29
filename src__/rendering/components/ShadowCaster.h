#pragma once

#include <ecs.h>

struct ShadowCaster : ecs::ComponentOf<ShadowCaster> {
    explicit ShadowCaster(bool casts = true) : casts_shadows(casts) {}

    bool casts_shadows{true};
};
