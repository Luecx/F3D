//
// Created by Luecx on 23.06.2024.
//

#ifndef F3D_IMAGE_DATA_H
#define F3D_IMAGE_DATA_H

#include "resource_data.h"

struct ImageData : public ResourceData {
    using ResourceData::ResourceData;

    std::unique_ptr<unsigned char[]> cpu_data;
    TextureData::UPtr gpu_data;

    bool load_cpu_specific() override;
    bool load_gpu_specific() override;
    bool unload_cpu_specific() override;
    bool unload_gpu_specific() override;
};


#endif    // F3D_IMAGE_DATA_H
