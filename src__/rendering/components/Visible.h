#pragma once

#include <ecs.h>

struct Visibility : ecs::ComponentOf<Visibility> {
    explicit Visibility(bool enabled = true) : enabled(enabled) {}

    bool enabled{true};
};

using Visible = Visibility;
