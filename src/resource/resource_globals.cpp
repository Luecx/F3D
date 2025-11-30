#include "resource_globals.h"

#include "textures/texture_manager.h"
#include "materials/material_manager.h"
#include <iostream>

TextureManager& global_texture_manager() {
    static TextureManager manager;
    static bool once = [](){ std::cout << "Created global TextureManager\n"; return true; }();
    (void)once;
    return manager;
}

MaterialManager& global_material_manager() {
    static MaterialManager manager;
    static bool once = [](){ std::cout << "Created global MaterialManager\n"; return true; }();
    (void)once;
    return manager;
}
