#pragma once

#include <memory>
#include <string>

#include "../resource_base.h"
#include "../resource_state.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

#include "../textures/texture.h"
#include "material_properties.h"

/**
 * @brief Material resource with physically-based properties and texture slots.
 *
 * CPU state: MaterialProperties (always present, cheap).
 * GPU state: "logical" GPU state is:
 *   - All referenced textures are resident on the GPU.
 *   - A corresponding GPU_Material entry in a global SSBO (updated by MaterialManager).
 */
class Material : public ResourceBase, public std::enable_shared_from_this<Material> {
    public:
    using Ptr = std::shared_ptr<Material>;

    explicit Material(std::string name = {})
        : name_(std::move(name)) {
        properties_.set_defaults();
    }

    const std::string& name() const { return name_; }

    MaterialProperties&       properties()       { return properties_; }
    const MaterialProperties& properties() const { return properties_; }

    /// Returns whether this material needs its SSBO entry updated.
    bool gpu_dirty() const noexcept { return gpu_dirty_; }
    void clear_gpu_dirty() noexcept { gpu_dirty_ = false; }

    protected:
    void impl_load(ResourceState state) override {
        switch (state) {
            case ResourceState::Cpu:
                // Nothing special; properties_ are already alive.
                logging::log(reslog::MATERIAL, logging::DEBUG,
                             "CPU acquire material '" + name_ + "'");
                break;

            case ResourceState::Gpu:
                logging::log(reslog::MATERIAL, logging::INFO,
                             "GPU acquire material '" + name_ + "'");
                acquire_textures_gpu();
                gpu_dirty_ = true; // SSBO entry should be (re)built/updated
                break;

            case ResourceState::Drive:
            default:
                // Drive is conceptual only.
                break;
        }
    }

    void impl_unload(ResourceState state) override {
        switch (state) {
            case ResourceState::Cpu:
                logging::log(reslog::MATERIAL, logging::DEBUG,
                             "CPU release material '" + name_ + "'");
                // We typically keep properties_ in memory; no-op.
                break;

            case ResourceState::Gpu:
                logging::log(reslog::MATERIAL, logging::INFO,
                             "GPU release material '" + name_ + "'");
                release_textures_gpu();
                gpu_dirty_ = true; // If re-acquired, SSBO entry should be refreshed
                break;

            case ResourceState::Drive:
            default:
                break;
        }
    }

    private:
    void acquire_textures_gpu() {
        auto req_tex = [](const std::shared_ptr<Texture>& tex) {
            if (tex) tex->require(ResourceState::Gpu);
        };

        auto& p = properties_;
        req_tex(p.base_color.texture);
        req_tex(p.emission_color.texture);
        req_tex(p.sheen_color.texture);
        req_tex(p.subsurface_color.texture);
        req_tex(p.subsurface_radius.texture);

        req_tex(p.roughness.texture);
        req_tex(p.metallic.texture);
        req_tex(p.specular.texture);
        req_tex(p.specular_tint.texture);
        req_tex(p.transmission.texture);
        req_tex(p.transmission_roughness.texture);
        req_tex(p.clearcoat.texture);
        req_tex(p.clearcoat_roughness.texture);
        req_tex(p.subsurface.texture);
        req_tex(p.sheen.texture);
        req_tex(p.sheen_tint.texture);
        req_tex(p.anisotropic.texture);
        req_tex(p.ior.texture);
        req_tex(p.anisotropic_rotation.texture);
        req_tex(p.emission_strength.texture);

        req_tex(p.normal_map);
        req_tex(p.displacement_map);
        req_tex(p.ambient_occlusion_map);
    }

    void release_textures_gpu() {
        auto rel_tex = [](const std::shared_ptr<Texture>& tex) {
            if (tex) tex->release(ResourceState::Gpu);
        };

        auto& p = properties_;
        rel_tex(p.base_color.texture);
        rel_tex(p.emission_color.texture);
        rel_tex(p.sheen_color.texture);
        rel_tex(p.subsurface_color.texture);
        rel_tex(p.subsurface_radius.texture);

        rel_tex(p.roughness.texture);
        rel_tex(p.metallic.texture);
        rel_tex(p.specular.texture);
        rel_tex(p.specular_tint.texture);
        rel_tex(p.transmission.texture);
        rel_tex(p.transmission_roughness.texture);
        rel_tex(p.clearcoat.texture);
        rel_tex(p.clearcoat_roughness.texture);
        rel_tex(p.subsurface.texture);
        rel_tex(p.sheen.texture);
        rel_tex(p.sheen_tint.texture);
        rel_tex(p.anisotropic.texture);
        rel_tex(p.ior.texture);
        rel_tex(p.anisotropic_rotation.texture);
        rel_tex(p.emission_strength.texture);

        rel_tex(p.normal_map);
        rel_tex(p.displacement_map);
        rel_tex(p.ambient_occlusion_map);
    }

    std::string        name_;
    MaterialProperties properties_;
    bool               gpu_dirty_{true}; // start dirty so first upload initializes it
};
