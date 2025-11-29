#include "mesh.h"

#include <iostream>
#include <utility>

Mesh::Mesh(std::string path)
    : path_(std::move(path)) {}

void Mesh::request_state(resources::ResourceState state) {
    using S = resources::ResourceState;

    if (state == S::Drive) {
        // Drive is implicit; nothing to do.
        return;
    }

    if (state == S::Ram) {
        if (!has_ram_) {
            if (!load_from_drive()) {
                std::cerr << "Mesh::request_state(Ram) failed for file: " << path_ << '\n';
                return;
            }
            has_ram_ = true;
        }
        return;
    }
}

void Mesh::release_state(resources::ResourceState state) {
    using S = resources::ResourceState;

    if (state == S::Drive) {
        return;
    }

    if (state == S::Ram) {
        if (has_ram_) {
            free_ram();
            has_ram_ = false;
        }
        return;
    }
}

bool Mesh::is_in_state(resources::ResourceState state) const {
    using S = resources::ResourceState;

    if (state == S::Drive) {
        return true;
    }
    if (state == S::Ram) {
        return has_ram_;
    }
    return false;
}

bool Mesh::load_from_drive() {
    // TODO: implement actual mesh loading here (OBJ, GLTF, etc.).
    // For now we just allocate an empty CPU data object so that the
    // pipeline compiles and can be wired up step-by-step.

    cpu_data_ = std::make_shared<MeshCPUData>();

    // In a real implementation, failure to parse would return false.
    return true;
}

void Mesh::free_ram() {
    cpu_data_.reset();
}
