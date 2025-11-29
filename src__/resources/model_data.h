//
// Created by Luecx on 06.04.2025.
//

#ifndef MODEL_DATA_H
#define MODEL_DATA_H

#include "resource_data.h"

class ModelData : public ResourceData {
  public:
    using ResourceData::ResourceData;

  protected:
    bool load_to_ram() override;
    void unload_from_ram() override;
    bool load_to_gpu() override;
    void unload_from_gpu() override;
};

#endif // MODEL_DATA_H
