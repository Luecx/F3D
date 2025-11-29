#ifndef MATERIAL_DATA_H
#define MATERIAL_DATA_H

#include "resource_data.h"
#include "../material/material.h"

#include <string>

class MaterialData : public ResourceData {
  public:
    MaterialData(const std::string& path, std::string material_name);

    bool load_to_ram() override;
    void unload_from_ram() override;
    bool load_to_gpu() override;
    void unload_from_gpu() override;

    Material::SPtr material;
    Material* get_material() const { return material.get(); }
    int gpu_material_index() const { return gpu_material_index_; }
    bool is_transparent(float threshold = 0.001f) const;

    const std::string& material_name() const { return material_name_; }

  private:
    std::string material_name_;
    int gpu_material_index_{-1};
};

#endif // MATERIAL_DATA_H
