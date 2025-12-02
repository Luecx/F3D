#pragma once

#include <cstddef>
#include <memory>

class Material;

/**
 * @brief Represents a sub-range of a mesh's index buffer with an associated material.
 *
 * Purely CPU-side information:
 *  - index_offset / index_count describe a range in MeshCPUData::indices.
 *  - material is a shared_ptr<Material> for rendering.
 */
struct Submesh {
    /// First index within the mesh's index buffer.
    std::size_t index_offset{0};

    /// Number of indices belonging to this submesh.
    std::size_t index_count{0};

    /// Material referenced by this submesh. May be nullptr for a default material.
    std::shared_ptr<Material> material;
};
