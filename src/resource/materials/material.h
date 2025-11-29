#pragma once

#include <memory>
#include <string>

#include "material_properties.h"
#include "../resource_types.h"

/**
 * @brief High-level material object used throughout the engine.
 *
 * A Material owns a @ref MaterialProperties instance (constants + texture
 * references) and provides convenience helpers for managing the *states* of
 * the textures it references.
 *
 * Importantly, a Material does not directly manage any GPU buffers itself.
 * The @ref MaterialManager is responsible for converting a Material into the
 * GPU_Material struct and placing it in an SSBO. However, you can still call
 * request_state() on a Material to request that all of its textures be loaded
 * to RAM or GPU.
 */
class Material {
  public:
    using SPtr = std::shared_ptr<Material>;

    /**
     * @brief Construct a material with an optional human-readable name.
     */
    explicit Material(std::string name = {});

    /**
     * @brief Get the name of the material.
     */
    [[nodiscard]] const std::string& name() const { return name_; }

    /**
     * @brief Set the name of the material.
     */
    void set_name(std::string name) { name_ = std::move(name); }

    /**
     * @brief Access the material properties (mutable).
     */
    [[nodiscard]] MaterialProperties& properties() { return properties_; }

    /**
     * @brief Access the material properties (read-only).
     */
    [[nodiscard]] const MaterialProperties& properties() const { return properties_; }

    /**
     * @brief Initialize this material with default principled values.
     *
     * This simply forwards to MaterialProperties::set_defaults().
     */
    void set_default_properties() { properties_.set_defaults(); }

    /**
     * @brief Request a resource state for this material.
     *
     * This does not load anything itself; instead it forwards the request to
     * all textures referenced by @ref MaterialProperties. For example:
     *
     *  - request_state(ResourceState::Ram) will request RAM for all textures.
     *  - request_state(ResourceState::Gpu) will request GPU for all textures.
     *
     * @param state Target resource state (Drive, Ram, or Gpu).
     */
    void request_state(resources::ResourceState state);

    /**
     * @brief Release a previously requested resource state for this material.
     *
     * This forwards the release to all referenced textures.
     *
     * @param state Target resource state (Ram or Gpu).
     */
    void release_state(resources::ResourceState state);

    /**
     * @brief Query whether this material is effectively in @p state.
     *
     * Since a Material is an aggregate of textures, the state is derived
     * from the states of all referenced textures:
     *  - Drive: always considered true.
     *  - Ram: true if every texture is at least in Ram or Gpu.
     *  - Gpu: true if every texture is in Gpu.
     *
     * If the material references no textures, this returns true for all
     * states (since constants are always available).
     */
    [[nodiscard]] bool is_in_state(resources::ResourceState state) const;

  private:
    std::string        name_;
    MaterialProperties properties_;
};
