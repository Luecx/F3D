#pragma once

#include "../resources/mesh_data.h"
#include "../material/material.h"

#include <memory>
#include <string>
#include <vector>

class Model {
  public:
    struct MeshEntry {
        std::shared_ptr<MeshData> mesh;
        std::vector<std::shared_ptr<Material>> materials;

        bool valid() const { return mesh != nullptr && !materials.empty(); }
    };

    using SPtr = std::shared_ptr<Model>;

    explicit Model(std::string name = {});

    void add_mesh_entry(const std::shared_ptr<MeshData>& mesh, std::vector<std::shared_ptr<Material>> materials);

    const std::vector<MeshEntry>& mesh_entries() const;
    const std::string& name() const;

    std::size_t mesh_count() const;
    std::size_t material_count() const;

    bool valid() const;

  private:
    std::string name_;
    std::vector<MeshEntry> mesh_entries_;
};
