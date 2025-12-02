#include "material_manager.h"
#include "../../logging/logging.h"

// ------------------------------------------------------------
// Material lookup / creation
// ------------------------------------------------------------

std::shared_ptr<Material>
    MaterialManager::get_or_create(const std::string& name)
{
    auto it = materials_.find(name);
    if (it != materials_.end()) {
        return it->second;
    }

    auto mat = std::make_shared<Material>();
    materials_[name] = mat;
    return mat;
}

std::shared_ptr<Material>
    MaterialManager::get(const std::string& name) const
{
    auto it = materials_.find(name);
    if (it != materials_.end()) {
        return it->second;
    }
    return nullptr;
}

void MaterialManager::set(const std::string& name,
                          const std::shared_ptr<Material>& mat)
{
    materials_[name] = mat;
}


// ------------------------------------------------------------
// Material indexing
// ------------------------------------------------------------

std::uint32_t MaterialManager::get_or_assign_index(
    const std::shared_ptr<Material>& mat)
{
    if (!mat) return 0;

    Material* raw = mat.get();

    // Already assigned?
    auto it = material_to_index_.find(raw);
    if (it != material_to_index_.end()) {
        return it->second;
    }

    // Assign a new index
    std::uint32_t idx = static_cast<std::uint32_t>(ordered_.size());
    ordered_.push_back(mat);
    material_to_index_[raw] = idx;

    return idx;
}

std::uint32_t MaterialManager::try_get_index(
    const std::shared_ptr<Material>& mat) const
{
    if (!mat) return UINT32_MAX;

    Material* raw = mat.get();
    auto it = material_to_index_.find(raw);
    if (it == material_to_index_.end()) {
        return UINT32_MAX;
    }
    return it->second;
}


// ------------------------------------------------------------
// GPU Upload
// ------------------------------------------------------------

void MaterialManager::update_gpu_materials()
{
    if (ordered_.empty()) {
        logging::log("MATERIAL", logging::WARNING,
                     "MaterialManager: update_gpu_materials but no materials exist.");
        return;
    }

    // Build CPU buffer containing all material GPU data
    std::vector<MaterialGPU> gpu_data;
    gpu_data.reserve(ordered_.size());

    for (auto& mat : ordered_) {
        gpu_data.push_back(mat->to_gpu_struct());
    }

    const GLsizeiptr byte_size = static_cast<GLsizeiptr>(
        gpu_data.size() * sizeof(MaterialGPU));

    if (!material_ssbo_) {
        material_ssbo_ = std::make_shared<SSBOData>();
    }

    material_ssbo_->bind();
    material_ssbo_->allocate(byte_size, gpu_data.data());
    material_ssbo_->unbind();

    logging::log("MATERIAL", logging::INFO,
                 "Uploaded " + std::to_string(ordered_.size()) +
                     " materials to SSBO (" + std::to_string(byte_size) +
                     " bytes).");
}
