#include "material_manager.h"

#include <algorithm>
#include <vector>

#include "../textures/texture.h"
#include "../../logging/logging.h"
#include "../resource_logging.h"

namespace {
void collect_textures(const MaterialProperties& props,
                      std::vector<std::shared_ptr<Texture>>& out) {
    auto collect_color = [&](const ColorComponent& comp) {
        if (comp.mode == ComponentMode::TEXTURE && comp.texture) out.push_back(comp.texture);
    };
    auto collect_float = [&](const FloatComponent& comp) {
        if (comp.mode == ComponentMode::TEXTURE && comp.texture) out.push_back(comp.texture);
    };

    collect_color(props.base_color);
    collect_color(props.emission_color);
    collect_color(props.sheen_color);

    collect_float(props.roughness);
    collect_float(props.metallic);
    collect_float(props.specular);
    collect_float(props.specular_tint);
    collect_float(props.transmission);
    collect_float(props.transmission_roughness);
    collect_float(props.clearcoat);
    collect_float(props.clearcoat_roughness);
    collect_float(props.subsurface);
    collect_float(props.sheen);
    collect_float(props.sheen_tint);
    collect_float(props.anisotropic);
    collect_float(props.ior);
    collect_float(props.anisotropic_rotation);

    if (props.normal_map) out.push_back(props.normal_map);
    if (props.tangent_map) out.push_back(props.tangent_map);
}
} // namespace

using resources::ResourceState;

MaterialManager::MaterialManager()
    : _allocated_count(0) {} // SSBO allocated lazily in update_gpu_buffer()

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
            mat->request(ResourceState::Gpu);
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

    // 4) Bind to the same binding point used by your shaders, but only if SSBOs are supported.
#ifdef GL_SHADER_STORAGE_BUFFER
    _ssbo_buffer->bind(3);
#endif
}

std::shared_ptr<SSBOData> MaterialManager::get_ssbo() const {
    return _ssbo_buffer;
}

void MaterialManager::dump_state(int indent) const {
    const std::string pad(indent, ' ');
    logging::log(reslog::MATERIAL, logging::INFO, pad + "MaterialManager state:");
    for (std::size_t i = 0; i < _materials.size(); ++i) {
        auto mat = _materials[i];
        if (!mat) continue;
        const auto name = mat->name().empty() ? std::string("<unnamed>") : mat->name();
        const bool inRam = mat->is_in_state(resources::ResourceState::Ram);
        const bool inGpu = mat->is_in_state(resources::ResourceState::Gpu);
        logging::log(reslog::MATERIAL, logging::INFO,
                     pad + "  [" + std::to_string(i) + "] " + name +
                         " RAM=" + (inRam ? "yes" : "no") +
                         " GPU=" + (inGpu ? "yes" : "no"));

        std::vector<std::shared_ptr<Texture>> textures;
        // reuse helper
        {
            collect_textures(mat->properties(), textures);
        }
        for (const auto& tex : textures) {
            if (!tex) continue;
            logging::log(reslog::MATERIAL, logging::INFO,
                         pad + "    tex " + tex->path() +
                             " RAM=" + (tex->has_ram() ? "yes" : "no") +
                             " GPU=" + (tex->has_gpu() ? "yes" : "no") +
                             " rc_ram=" + std::to_string(tex->ram_refcount()) +
                             " rc_gpu=" + std::to_string(tex->gpu_refcount()));
        }
    }
}
