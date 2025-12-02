#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#include "material.h"
#include "../../gldata/ssbo_data.h"

/**
 * @brief Global material manager holding:
 *   - All material objects (name → material)
 *   - Ordering of materials packed into the global SSBO
 *   - Pointer → index mapping for stable SSBO indices
 *
 * Responsibilities:
 *   - Own creation/find of Materials
 *   - Own mapping of Material → SSBO index
 *   - Pack all materials into the global GPU SSBO
 */
class MaterialManager {
    public:
    using Ptr = std::shared_ptr<MaterialManager>;

    MaterialManager() = default;

    // ------------------------------------------------------------
    // Material lookup / creation
    // ------------------------------------------------------------

    /// Returns existing material or creates a new one.
    std::shared_ptr<Material> get_or_create(const std::string& name);

    /// Returns existing material or nullptr.
    std::shared_ptr<Material> get(const std::string& name) const;

    /// Add or override a material explicitly.
    void set(const std::string& name, const std::shared_ptr<Material>& mat);

    // ------------------------------------------------------------
    // Material indexing for SSBO
    // ------------------------------------------------------------

    /**
     * @brief Assign (or return existing) stable index for this material.
     *
     * This index corresponds to an entry in `ordered_` and in the SSBO.
     */
    std::uint32_t get_or_assign_index(const std::shared_ptr<Material>& mat);

    /**
     * @brief Retrieve an index WITHOUT assigning one.
     * Returns UINT32_MAX if not found.
     */
    std::uint32_t try_get_index(const std::shared_ptr<Material>& mat) const;

    // ------------------------------------------------------------
    // GPU operations
    // ------------------------------------------------------------

    /// Upload all materials in ordered_ into the SSBO.
    void update_gpu_materials();

    /// Returns the GPU SSBO pointer (may be nullptr).
    const std::shared_ptr<SSBOData>& ssbo() const { return material_ssbo_; }

    /// Number of materials packed into SSBO.
    std::size_t material_count() const { return ordered_.size(); }

    private:
    // All materials: name → material
    std::unordered_map<std::string, std::shared_ptr<Material>> materials_;

    // Material table for SSBO upload: stable identity by index
    std::vector<std::shared_ptr<Material>> ordered_;

    // Reverse mapping: material pointer → index in ordered_
    std::unordered_map<Material*, std::uint32_t> material_to_index_;

    // GPU material buffer
    std::shared_ptr<SSBOData> material_ssbo_;
};
