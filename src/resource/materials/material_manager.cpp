#include "material_manager.h"

#include <algorithm>
#include <vector>

#include "../textures/texture.h"

using resources::ResourceState;

MaterialManager::MaterialManager()
    : _allocated_count(1024) { // initial capacity
    _ssbo_buffer = std::make_shared<SSBOData>();

    std::vector<GPU_Material> initial(_allocated_count);
    _ssbo_buffer->update_data(_allocated_count * sizeof(GPU_Material), initial.data());

    // Example binding point; must match mat.glsl:
    // layout(std430, binding = 3) buffer MaterialBuffer { GPU_Material materials[]; };
    _ssbo_buffer->bind(3);
}

MaterialManager::~MaterialManager() = default;

std::size_t MaterialManager::add_material(const std::shared_ptr<Material>& material) {
    if (!material) {
        return static_cast<std::size_t>(-1);
    }

    _materials.push_back(material);
    const std::size_t index = _materials.size() - 1;

    if (_materials.size() > _allocated_count) {
        // Grow capacity exponentially to avoid frequent reallocations.
        _allocated_count = std::max<std::size_t>(_allocated_count * 2, _materials.size());
    }

    // You may choose to defer this and call update_gpu_buffer() manually later.
    update_gpu_buffer();
    return index;
}

std::shared_ptr<Material> MaterialManager::get_material(std::size_t index) const {
    if (index >= _materials.size()) {
        return nullptr;
    }
    return _materials[index];
}

GPU_ColorComponent MaterialManager::convert_to_gpu_color_component(const ColorComponent& comp) const {
    GPU_ColorComponent gpu{};
    gpu.enabled        = 0;
    gpu.color[0]       = comp.r;
    gpu.color[1]       = comp.g;
    gpu.color[2]       = comp.b;
    gpu.texture_handle = 0;

    if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
        if (comp.texture->is_in_state(ResourceState::Gpu)) {
            if (auto* tex_gpu = comp.texture->gpu_data()) {
                gpu.enabled        = 1;
                gpu.texture_handle = tex_gpu->get_handle();
            }
        }
    }

    return gpu;
}

GPU_ScalarComponent MaterialManager::convert_to_gpu_scalar_component(const FloatComponent& comp) const {
    GPU_ScalarComponent gpu{};
    gpu.enabled        = 0;
    gpu.value          = comp.value;
    gpu.texture_handle = 0;

    if (comp.mode == ComponentMode::TEXTURE && comp.texture) {
        if (comp.texture->is_in_state(ResourceState::Gpu)) {
            if (auto* tex_gpu = comp.texture->gpu_data()) {
                gpu.enabled        = 1;
                gpu.texture_handle = tex_gpu->get_handle();
            }
        }
    }

    return gpu;
}

GPU_TextureComponent MaterialManager::convert_to_gpu_texture_component(const std::shared_ptr<Texture>& tex) const {
    GPU_TextureComponent gpu{};
    gpu.enabled        = 0;
    gpu.texture_handle = 0;
    gpu.padding1       = 0;
    gpu.padding2       = 0;
    gpu.padding3       = 0;

    if (!tex) {
        return gpu;
    }

    if (tex->is_in_state(ResourceState::Gpu)) {
        if (auto* gpu_data = tex->gpu_data()) {
            gpu.enabled        = 1;
            gpu.texture_handle = gpu_data->get_handle();
        }
    }

    return gpu;
}

GPU_Material MaterialManager::convert_to_gpu_material(const Material& mat) const {
    GPU_Material gpu_mat{};

    const auto& props = mat.properties();

    // --- Color components ---
    gpu_mat.base_color  = convert_to_gpu_color_component(props.base_color);
    gpu_mat.emission    = convert_to_gpu_color_component(props.emission_color);
    gpu_mat.sheen_color = convert_to_gpu_color_component(props.sheen_color);

    // --- Scalar components ---
    gpu_mat.roughness              = convert_to_gpu_scalar_component(props.roughness);
    gpu_mat.metallic               = convert_to_gpu_scalar_component(props.metallic);
    gpu_mat.specular               = convert_to_gpu_scalar_component(props.specular);
    gpu_mat.specular_tint          = convert_to_gpu_scalar_component(props.specular_tint);
    gpu_mat.transmission           = convert_to_gpu_scalar_component(props.transmission);
    gpu_mat.transmission_roughness = convert_to_gpu_scalar_component(props.transmission_roughness);
    gpu_mat.clearcoat              = convert_to_gpu_scalar_component(props.clearcoat);
    gpu_mat.clearcoat_roughness    = convert_to_gpu_scalar_component(props.clearcoat_roughness);
    gpu_mat.subsurface             = convert_to_gpu_scalar_component(props.subsurface);
    gpu_mat.sheen                  = convert_to_gpu_scalar_component(props.sheen);
    gpu_mat.sheen_tint             = convert_to_gpu_scalar_component(props.sheen_tint);
    gpu_mat.anisotropy             = convert_to_gpu_scalar_component(props.anisotropic);

    // Scalars used directly as floats:
    gpu_mat.ior                 = props.ior.value;
    gpu_mat.anisotropy_rotation = props.anisotropic_rotation.value;
    gpu_mat.padding1            = 0.0f;
    gpu_mat.padding2            = 0.0f;

    // Texture-only slots:
    gpu_mat.normal_map  = convert_to_gpu_texture_component(props.normal_map);
    gpu_mat.tangent_map = convert_to_gpu_texture_component(props.tangent_map);

    return gpu_mat;
}

void MaterialManager::update_gpu_buffer() {
    if (!_ssbo_buffer) {
        _ssbo_buffer = std::make_shared<SSBOData>();
    }

    // 1) Request GPU state for all materials (which forwards to all textures).
    for (auto& mat : _materials) {
        if (mat) {
            mat->request_state(ResourceState::Gpu);
        }
    }

    // 2) Convert all existing materials to GPU layout.
    std::vector<GPU_Material> gpu_materials;
    gpu_materials.reserve(_allocated_count);

    for (const auto& mat_ptr : _materials) {
        if (mat_ptr) {
            gpu_materials.push_back(convert_to_gpu_material(*mat_ptr));
        } else {
            gpu_materials.emplace_back(GPU_Material{});
        }
    }

    // 3) Zero-fill remaining capacity.
    gpu_materials.resize(_allocated_count);

    _ssbo_buffer->update_data(_allocated_count * sizeof(GPU_Material), gpu_materials.data());

    // 4) Bind to the same binding point used by your shaders.
    _ssbo_buffer->bind(3);
}

std::shared_ptr<SSBOData> MaterialManager::get_ssbo() const {
    return _ssbo_buffer;
}
