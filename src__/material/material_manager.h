#pragma once

#include <vector>
#include <memory>
#include "material.h"
#include "material_gpu.h"
#include "../gldata/ssbo_data.h"
#include "../resources/texture_resource.h"

class MaterialManager {
  public:
    MaterialManager();
    ~MaterialManager();

    // Add or remove a material.
    std::size_t add_material(const std::shared_ptr<Material>& material);
    void remove_material(size_t index);

    // Access a material.
    std::shared_ptr<Material> get_material(size_t index) const;
    size_t get_material_count() const;

    // Update the GPU buffer with the converted material data.
    // This will update the SSBO and bind it to binding point 3.
    void update_gpu_materials();

    // Retrieve the SSBOData object (if you need to bind it later).
    std::shared_ptr<SSBOData> get_ssbo() const;

  private:
    // Our CPU-side storage of materials.
    std::vector<std::shared_ptr<Material>> _materials;

    // The SSBO wrapper for GPU data.
    std::shared_ptr<SSBOData> _ssbo_buffer;

    // The current allocated capacity (number of materials) in the SSBO.
    size_t _allocated_count;

    // Helper conversion functions from CPU Material components to GPU structs.
    GPU_ColorComponent convert_to_gpu_color_component(const ColorComponent& comp) const;
    GPU_ScalarComponent convert_to_gpu_scalar_component(const FloatComponent& comp) const;
    GPU_TextureComponent convert_to_gpu_texture_component(const std::shared_ptr<TextureResource>& tex) const;
    GPU_Material convert_to_gpu_material(const Material& mat) const;
};
