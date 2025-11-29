#include "model.h"

Model::Model(std::string name) : name_(std::move(name)) {}

void Model::add_mesh_entry(const std::shared_ptr<MeshData>& mesh, std::vector<std::shared_ptr<Material>> materials) {
    mesh_entries_.push_back(MeshEntry{mesh, std::move(materials)});
}

const std::vector<Model::MeshEntry>& Model::mesh_entries() const { return mesh_entries_; }

const std::string& Model::name() const { return name_; }

std::size_t Model::mesh_count() const { return mesh_entries_.size(); }

std::size_t Model::material_count() const {
    std::size_t count = 0;
    for (const auto& entry : mesh_entries_) {
        count += entry.materials.size();
    }
    return count;
}

bool Model::valid() const {
    if (mesh_entries_.empty()) {
        return false;
    }
    for (const auto& entry : mesh_entries_) {
        if (!entry.valid()) {
            return false;
        }
    }
    return true;
}
