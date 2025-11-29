#pragma once

#include <ecs.h>

struct Transparency : ecs::ComponentOf<Transparency> {
    Transparency() = default;
    explicit Transparency(bool enabled) : enabled(enabled) {}

    bool enabled{true};
};
