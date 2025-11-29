#pragma once

#include "LitShader.h"

class LitTransparentShader : public LitShader {
public:
    LitTransparentShader();

    bool init(const std::filesystem::path& shader_dir);
};
