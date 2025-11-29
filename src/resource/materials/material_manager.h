#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "material_gpu.h"
#include "material_properties.h"
#include "material.h"

#include "../resource_types.h"
#include "../gldata/ssbo_data.h"

class Texture;

/**
 * @brief Manages a list of materials and their GPU representation.
 *
 * MaterialManager is responsible for:
 *  - Owning a list of @ref Material objects.
 *  - Converting each Material into @ref GPU_Material.
 *  - Packing these GPU_Material structs into a single SSBO.
 *
 * It does NOT parse files or handle higher-level resource lookup; that is
 * left to your resource / asset systems. This class only deals with:
 *
 *  - CPU material objects.
 *  - Their GPU-packed representation.
 *  - An SSBO bound at a fixed binding point.
 */
class MaterialManager {
  public:
    /**
     * @brief Construct the manager and allocate an initial SSBO capacity.
     *
     * The SSBO is created with an initial capacity (e.g. 1024 materials)
     * and bound to the binding point that matches mat.glsl.
     */
    MaterialManager();

    /**
     * @brief Destroy the manager. GPU buffers are released automatically
     *        via SSBOData RAII.
     */
    ~MaterialManager();

    /**
     * @brief Add a material to the manager and return its index.
     *
     * The material is appended to the internal list. If capacity is exceeded,
     * the internal buffers are grown on the next @ref update_gpu_buffer call.
     */
    std::size_t add_material(const std::shared_ptr<Material>& material);

    /**
     * @brief Retrieve a material by index (or nullptr if out of range).
     */
    [[nodiscard]] std::shared_ptr<Material> get_material(std::size_t index) const;

    /**
     * @brief Rebuild and upload the GPU representation of all materials.
     *
     * This method:
     *  - Requests GPU state for each material (which forwards to its textures).
     *  - Converts each Material into a GPU_Material struct.
     *  - Writes a tightly packed array to the SSBO, zero-filling unused slots.
     *
     * You can call this once after asset loading, or whenever material
     * properties / texture assignments change. For a few thousand materials,
     * rebuilding the buffer every frame is usually acceptable on desktop.
     */
    void update_gpu_buffer();

    /**
     * @brief Get the underlying SSBO that stores the GPU_Material array.
     *
     * This SSBO must be bound to the same binding point used by the GLSL
     * declaration in mat.glsl:
     *
     * layout(std430, binding = 3) buffer MaterialBuffer {
     *     GPU_Material materials[];
     * };
     */
    [[nodiscard]] std::shared_ptr<SSBOData> get_ssbo() const;

  private:
    // Conversion helpers:
    GPU_ColorComponent   convert_to_gpu_color_component(const ColorComponent& comp) const;
    GPU_ScalarComponent  convert_to_gpu_scalar_component(const FloatComponent& comp) const;
    GPU_TextureComponent convert_to_gpu_texture_component(const std::shared_ptr<Texture>& tex) const;
    GPU_Material         convert_to_gpu_material(const Material& mat) const;

  private:
    std::vector<std::shared_ptr<Material>> _materials;     ///< List of CPU materials.
    std::size_t                            _allocated_count; ///< Allocated GPU slots.
    std::shared_ptr<SSBOData>              _ssbo_buffer;   ///< SSBO holding GPU_Material array.
};
