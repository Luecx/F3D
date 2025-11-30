#pragma once

class TextureManager;
class MaterialManager;

/**
 * @brief Access to global texture manager singleton.
 */
TextureManager& global_texture_manager();

/**
 * @brief Access to global material manager singleton.
 */
MaterialManager& global_material_manager();
