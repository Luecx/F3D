#include "mesh_cpu_data.h"

bool MeshCPUData::is_valid() const {
    // Positions must be a multiple of 3 floats.
    if (positions.size() % 3 != 0) {
        return false;
    }

    const std::size_t v = vertex_count();

    // Normals, if present, must match vertex count (3 floats per vertex).
    if (!normals.empty() && normals.size() != v * 3) {
        return false;
    }

    // Texcoords, if present, must match vertex count (2 floats per vertex).
    if (!texcoords.empty() && texcoords.size() != v * 2) {
        return false;
    }

    // We require an index buffer for rendering.
    if (indices.empty()) {
        return false;
    }

    const std::size_t ic = index_count();
    for (const auto& sm : submeshes) {
        if (sm.index_offset + sm.index_count > ic) {
            return false;
        }
    }

    return true;
}
