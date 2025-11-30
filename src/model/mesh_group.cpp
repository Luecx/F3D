#include "mesh_group.h"

#include <algorithm>
#include "../resource/resource_logging.h"
#include "../logging/logging.h"
#include "../resource/materials/material.h"

void MeshGroup::add_mesh(const Mesh::SPtr& mesh) {
    meshes_.push_back(mesh);
}

void MeshGroup::remove_mesh(const Mesh::SPtr& mesh) {
    meshes_.erase(std::remove(meshes_.begin(), meshes_.end(), mesh), meshes_.end());
}

void MeshGroup::ensure_material_list() {
    materials_used_.clear();
    std::vector<std::shared_ptr<Material>> uniques;
    for (auto& mesh : meshes_) {
        if (!mesh) continue;
        auto cpu = mesh->cpu_data();
        if (!cpu) continue;
        for (const auto& sm : cpu->submeshes) {
            if (!sm.material) continue;
            // dedup
            bool exists = false;
            for (auto& mptr : uniques) {
                if (mptr == sm.material) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                uniques.push_back(sm.material);
            }
        }
    }
    materials_used_ = std::move(uniques);
}

void MeshGroup::request(
    resources::ResourceState state,
    const std::function<std::uint32_t(const std::shared_ptr<Material>&)>& resolve_material_id) {
    using S = resources::ResourceState;

    logging::log(reslog::MESH, logging::INFO, "MeshGroup request to " + std::string(resources::to_string(state)));

    if (state == S::Drive) {
        return;
    }

    if (state == S::Ram) {
        if (has_ram_) {
            logging::log(reslog::MESH, logging::DEBUG, "MeshGroup RAM already present");
            return;
        }
        for (auto& m : meshes_) {
            if (m) {
                m->request(S::Ram);
            }
        }
        ensure_material_list();
        for (auto& mat : materials_used_) {
            if (mat) {
                mat->request(S::Ram);
            }
        }
        has_ram_ = true;
        logging::log(reslog::MESH, logging::INFO, "MeshGroup RAM requested for all meshes/materials");
        return;
    }

    if (state == S::Gpu) {
        if (has_gpu_) {
            logging::log(reslog::MESH, logging::DEBUG, "MeshGroup GPU already present");
            return;
        }
        request(S::Ram, resolve_material_id);

        // Build GPU buffers and resolve material ids.
        gpu_data_ = std::make_shared<MeshGroupGPUData>();
        auto resolver = resolve_material_id;
        if (!resolver) {
            resolver = [](const std::shared_ptr<Material>&) { return 0u; };
        }
        if (!gpu_data_->rebuild_from_meshes(meshes_, resolver)) {
            logging::log(reslog::MESH, logging::ERROR, "MeshGroup::request(Gpu): rebuild_from_meshes failed");
            gpu_data_.reset();
            has_gpu_ = false;
            return;
        }

        for (auto& mat : materials_used_) {
            if (mat) {
                mat->request(S::Gpu);
            }
        }

        has_gpu_ = true;
        logging::log(reslog::MESH, logging::INFO, "MeshGroup GPU buffers built");
        return;
    }
}

void MeshGroup::release(resources::ResourceState state) {
    using S = resources::ResourceState;

    logging::log(reslog::MESH, logging::INFO, "MeshGroup release from " + std::string(resources::to_string(state)));

    if (state == S::Drive) {
        return;
    }

    if (state == S::Gpu) {
        if (!has_gpu_) return;
        if (gpu_data_) {
            gpu_data_->release_gpu();
        }
        gpu_data_.reset();
        for (auto& mat : materials_used_) {
            if (mat) {
                mat->release(S::Gpu);
            }
        }
        has_gpu_ = false;
        logging::log(reslog::MESH, logging::INFO, "MeshGroup GPU buffers released");
        return;
    }

    if (state == S::Ram) {
        if (!has_ram_) return;
        for (auto& m : meshes_) {
            if (m) {
                m->release(S::Ram);
            }
        }
        for (auto& mat : materials_used_) {
            if (mat) {
                mat->release(S::Ram);
            }
        }
        has_ram_ = false;
        logging::log(reslog::MESH, logging::INFO, "MeshGroup RAM released");
        return;
    }
}
