#include "material_manager.h"

#include <vector>
#include <algorithm>
#include <glad/glad.h>

#include "../resources/resource_types.h"

// --- Conversion Helpers ---

GPU_ColorComponent MaterialManager::convert_to_gpu_color_component(const ColorComponent& comp) const {
    GPU_ColorComponent gpu_comp{};
    if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
        comp.texture->require(resources::ResourceState::Gpu);
        if (auto* texture = comp.texture->texture()) {
            gpu_comp.enabled = 1;
            gpu_comp.texture_handle = texture->get_handle();
            gpu_comp.color[0] = gpu_comp.color[1] = gpu_comp.color[2] = 0.0f;
            return gpu_comp;
        }
    }

    gpu_comp.enabled = 0;
    gpu_comp.texture_handle = 0;
    gpu_comp.color[0] = comp.r;
    gpu_comp.color[1] = comp.g;
    gpu_comp.color[2] = comp.b;
    return gpu_comp;
}

GPU_ScalarComponent MaterialManager::convert_to_gpu_scalar_component(const FloatComponent& comp) const {
    GPU_ScalarComponent gpu_comp{};
    if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
        comp.texture->require(resources::ResourceState::Gpu);
        if (auto* texture = comp.texture->texture()) {
            gpu_comp.enabled = 1;
            gpu_comp.texture_handle = texture->get_handle();
            gpu_comp.value = 0.0f;
            return gpu_comp;
        }
    }

    gpu_comp.enabled = 0;
    gpu_comp.texture_handle = 0;
    gpu_comp.value = comp.value;
    return gpu_comp;
}

GPU_TextureComponent
MaterialManager::convert_to_gpu_texture_component(const std::shared_ptr<TextureResource>& tex) const {
    GPU_TextureComponent gpu_comp{};
    if (tex) {
        tex->require(resources::ResourceState::Gpu);
        if (auto* texture = tex->texture()) {
            gpu_comp.enabled = 1;
            gpu_comp.texture_handle = texture->get_handle();
        }
    }
    gpu_comp.padding1 = gpu_comp.padding2 = gpu_comp.padding3 = 0;
    return gpu_comp;
}

GPU_Material MaterialManager::convert_to_gpu_material(const Material& mat) const {
    GPU_Material gpu_mat;
    // Color components.
    const auto& props = mat.properties();

    gpu_mat.base_color = convert_to_gpu_color_component(props.base_color);
    gpu_mat.emission = convert_to_gpu_color_component(props.emission_color);
    // Default sheen_color.
    gpu_mat.sheen_color.enabled = 0;
    gpu_mat.sheen_color.texture_handle = 0;
    gpu_mat.sheen_color.color[0] = 1.0f;
    gpu_mat.sheen_color.color[1] = 1.0f;
    gpu_mat.sheen_color.color[2] = 1.0f;

    // Scalar components.
    gpu_mat.roughness = convert_to_gpu_scalar_component(props.roughness);
    gpu_mat.metallic = convert_to_gpu_scalar_component(props.metallic);
    gpu_mat.specular = convert_to_gpu_scalar_component(props.specular);
    gpu_mat.specular_tint = convert_to_gpu_scalar_component(props.specular_tint);
    gpu_mat.transmission = convert_to_gpu_scalar_component(props.transmission);
    gpu_mat.transmission_roughness = convert_to_gpu_scalar_component(props.transmission_roughness);
    gpu_mat.clearcoat = convert_to_gpu_scalar_component(props.clearcoat);
    gpu_mat.clearcoat_roughness = convert_to_gpu_scalar_component(props.clearcoat_roughness);
    gpu_mat.subsurface = convert_to_gpu_scalar_component(props.subsurface);
    gpu_mat.sheen = convert_to_gpu_scalar_component(props.sheen);
    gpu_mat.sheen_tint = convert_to_gpu_scalar_component(props.sheen_tint);
    gpu_mat.anisotropy = convert_to_gpu_scalar_component(props.anisotropic);

    // Direct float values.
    gpu_mat.ior = props.ior.value;
    gpu_mat.anisotropy_rotation = props.anisotropic_rotation.value;
    gpu_mat.padding1 = gpu_mat.padding2 = 0.0f;

    // Texture-only components.
    gpu_mat.normal_map = convert_to_gpu_texture_component(props.normal_map);
    // For tangent_map, use defaults.
    gpu_mat.tangent_map.enabled = 0;
    gpu_mat.tangent_map.texture_handle = 0;
    gpu_mat.tangent_map.padding1 = gpu_mat.tangent_map.padding2 = gpu_mat.tangent_map.padding3 = 0;

    return gpu_mat;
}

// --- Constructor / Destructor ---

MaterialManager::MaterialManager()
    : _allocated_count(1024) // initial capacity for 1024 materials
{
    _ssbo_buffer = std::make_shared<SSBOData>();

    // Optionally, you can initialize the SSBO with the full allocated capacity (as zeroed data)
    std::vector<GPU_Material> initial_data(_allocated_count);
    _ssbo_buffer->update_data(_allocated_count * sizeof(GPU_Material), initial_data.data());
    // Bind to binding point 3.
    _ssbo_buffer->bind(3);
}

MaterialManager::~MaterialManager() {
    // _ssbo_buffer gets automatically cleaned up.
}

// --- Material Modification Functions ---

std::size_t MaterialManager::add_material(const std::shared_ptr<Material>& material) {
    if (!material) {
        return static_cast<std::size_t>(-1);
    }

    auto existing = std::find(_materials.begin(), _materials.end(), material);
    if (existing != _materials.end()) {
        size_t index = static_cast<size_t>(std::distance(_materials.begin(), existing));
        GPU_Material gpu_mat = convert_to_gpu_material(**existing);
        GLintptr offset = static_cast<GLintptr>(index * sizeof(GPU_Material));
        _ssbo_buffer->update_data(sizeof(GPU_Material), &gpu_mat, offset, GL_DYNAMIC_DRAW);
        return index;
    }

    _materials.push_back(material);
    size_t index = _materials.size() - 1;

    // Check if we need to grow the allocated SSBO.
    if (_materials.size() > _allocated_count) {
        // Compute new capacity: round up to next multiple of 1024.
        size_t new_allocated = ((_materials.size() + 1023) / 1024) * 1024;
        _allocated_count = new_allocated;

        // Update the entire SSBO with new capacity.
        std::vector<GPU_Material> gpu_materials;
        gpu_materials.reserve(_materials.size());
        for (const auto& mat_ptr : _materials) {
            if (mat_ptr) {
                gpu_materials.push_back(convert_to_gpu_material(*mat_ptr));
            }
        }
        // Zero-fill the remaining capacity if desired.
        gpu_materials.resize(_allocated_count);
        _ssbo_buffer->update_data(_allocated_count * sizeof(GPU_Material), gpu_materials.data());
    } else {
        // Otherwise, update only the new material via sub-data update.
        GPU_Material gpu_mat = convert_to_gpu_material(*material);
        GLintptr offset = static_cast<GLintptr>(index * sizeof(GPU_Material));
        _ssbo_buffer->update_data(sizeof(GPU_Material), &gpu_mat, offset, GL_DYNAMIC_DRAW);
    }

    return index;
}

void MaterialManager::remove_material(size_t index) {
    if (index < _materials.size()) {
        _materials.erase(_materials.begin() + index);
        // Removal changes indices so a full update is prudent.
        update_gpu_materials();
    }
}

std::shared_ptr<Material> MaterialManager::get_material(size_t index) const {
    return (index < _materials.size()) ? _materials[index] : nullptr;
}

size_t MaterialManager::get_material_count() const { return _materials.size(); }

// --- GPU Buffer Update ---

// Fully update the GPU buffer with all material data and rebind to binding point 3.
void MaterialManager::update_gpu_materials() {
    std::vector<GPU_Material> gpu_materials;
    gpu_materials.reserve(_materials.size());
    for (const auto& mat_ptr : _materials) {
        if (mat_ptr) {
            gpu_materials.push_back(convert_to_gpu_material(*mat_ptr));
        }
    }
    // If our current allocation exceeds the material count, zero-fill the remaining space.
    gpu_materials.resize(_allocated_count);

    _ssbo_buffer->update_data(_allocated_count * sizeof(GPU_Material), gpu_materials.data());
    _ssbo_buffer->bind(3);
}

// Return the SSBOData instance.
std::shared_ptr<SSBOData> MaterialManager::get_ssbo() const { return _ssbo_buffer; }
