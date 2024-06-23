#include "mesh_data.h"

bool MeshData::load_cpu_specific() {
    // Mesh-specific CPU loading logic here
    return true;
}

bool MeshData::load_gpu_specific() {
    // Mesh-specific GPU loading logic here
    return true;
}

bool MeshData::unload_cpu_specific() {
    // Mesh-specific CPU unloading logic here
    return true;
}

bool MeshData::unload_gpu_specific() {
    // Mesh-specific GPU unloading logic here
    return true;
}
