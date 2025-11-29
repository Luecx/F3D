#include "mesh_group.h"

#include <algorithm>
#include <iostream>

void MeshGroup::add_mesh(const Mesh::SPtr& mesh) {
    meshes_.push_back(mesh);
}

void MeshGroup::remove_mesh(const Mesh::SPtr& mesh) {
    meshes_.erase(std::remove(meshes_.begin(), meshes_.end(), mesh), meshes_.end());
}

void MeshGroup::request_state(resources::ResourceState state) {
    using S = resources::ResourceState;

    if (state == S::Drive) {
        return;
    }

    if (state == S::Ram) {
        for (auto& m : meshes_) {
            if (m) {
                m->request_state(S::Ram);
            }
        }
        has_ram_ = true;
        return;
    }

    if (state == S::Gpu) {
        // Ensure all meshes are in RAM first.
        for (auto& m : meshes_) {
            if (m) {
                m->request_state(S::Ram);
            }
        }
        has_ram_ = true;

        // Build unified GPU buffers for the group.
        gpu_data_ = std::make_shared<MeshGroupGPUData>();
        if (!gpu_data_->rebuild_from_meshes(meshes_)) {
            std::cerr << "MeshGroup::request_state(Gpu): rebuild_from_meshes failed\n";
            gpu_data_.reset();
            has_gpu_ = false;
            return;
        }

        has_gpu_ = true;
        return;
    }
}

void MeshGroup::release_state(resources::ResourceState state) {
    using S = resources::ResourceState;

    if (state == S::Drive) {
        return;
    }

    if (state == S::Gpu) {
        if (has_gpu_) {
            if (gpu_data_) {
                gpu_data_->release_gpu();
            }
            gpu_data_.reset();
            has_gpu_ = false;
        }
        return;
    }

    if (state == S::Ram) {
        for (auto& m : meshes_) {
            if (m) {
                m->release_state(S::Ram);
            }
        }
        has_ram_ = false;
        return;
    }
}

bool MeshGroup::is_in_state(resources::ResourceState state) const {
    using S = resources::ResourceState;

    if (state == S::Drive) {
        return true;
    }
    if (state == S::Ram) {
        return has_ram_;
    }
    if (state == S::Gpu) {
        return has_gpu_;
    }
    return false;
}
