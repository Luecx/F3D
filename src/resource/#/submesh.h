#pragma once

#include <cstddef>
#include <memory>

class Material;

/**
 * @brief Represents a sub-range of a mesh's index buffer together with a material.
 *
 * A Submesh is purely CPU-side information. It stores:
 *  - @ref index_offset : first index in the mesh's index buffer.
 *  - @ref index_count  : number of indices in this submesh.
 *  - @ref material     : the material assigned to this submesh.
 *
 * Submeshes do not contain any GPU data. They are consumed later by
 * MeshGroupGPUData when building the aggregated draw list for a chunk.
 */
struct Submesh {
    /// First index within the mesh's index buffer.
    std::size_t index_offset{0};

    /// Number of indices that belong to this submesh.
    std::size_t index_count{0};

    /// Material referenced by this submesh. May be nullptr for a default material.
    std::shared_ptr<Material> material;
};
