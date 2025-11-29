#pragma once

#include <vector>

#include "components/ModelComponent.h"
#include "components/Instances.h"
#include "components/ShadowCaster.h"
#include "components/Transparency.h"
#include "components/Visible.h"

struct RenderableInstance {
    ModelComponent* model{nullptr};
    Instances* instances{nullptr};
    Visibility* visibility{nullptr};
    ShadowCaster* shadow{nullptr};
    Transparency* transparency{nullptr};
    bool transparent{false};
};

using RenderableList = std::vector<RenderableInstance>;
