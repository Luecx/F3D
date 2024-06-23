#ifndef MESHDATA_H
#define MESHDATA_H

#include "resource_data.h"

struct MeshData : public ResourceData {
    using ResourceData::ResourceData;

    bool load_cpu_specific() override;
    bool load_gpu_specific() override;
    bool unload_cpu_specific() override;
    bool unload_gpu_specific() override;
};

#endif // MESHDATA_H
