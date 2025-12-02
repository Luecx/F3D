#pragma once

#include <cstdint>
#include <vector>

#include "submesh.h"

/**
 * @brief Pure CPU-side mesh data: attributes + indices + submeshes.
 *
 * This struct is:
 *  - Format-agnostic (can be filled from OBJ, GLTF, custom loaders, etc.).
 *  - Free of GPU/OpenGL concepts.
 */
struct MeshCPUData {
    // ---------------------------------------------------------------------
    // Vertex attributes
    // ---------------------------------------------------------------------

    /// Positions: 3 floats per vertex (x, y, z).
    std::vector<float> positions;

    /// Normals: optional, 3 floats per vertex (nx, ny, nz).
    std::vector<float> normals;

    /// Texture coordinates: optional, 2 floats per vertex (u, v).
    std::vector<float> texcoords;

    // ---------------------------------------------------------------------
    // Index buffer
    // ---------------------------------------------------------------------

    /// Triangle indices, referencing vertices by index.
    std::vector<std::uint32_t> indices;

    // ---------------------------------------------------------------------
    // Submeshes
    // ---------------------------------------------------------------------

    /**
     * @brief Logical submeshes that partition the index buffer.
     *
     * Each Submesh defines:
     *  - index_offset / index_count: a range in @ref indices
     *  - a Material pointer that should be used when rendering that range
     */
    std::vector<Submesh> submeshes;

    // ---------------------------------------------------------------------
    // Convenience queries
    // ---------------------------------------------------------------------

    /// Number of vertices (assuming positions are packed triples).
    std::size_t vertex_count() const { return positions.size() / 3; }

    /// Number of indices.
    std::size_t index_count() const { return indices.size(); }

    /**
     * @brief Checks semantic correctness of the mesh data.
     *
     * Requirements:
     *  - positions.size() % 3 == 0
     *  - normals.size()   == vertex_count() * 3 (or empty)
     *  - texcoords.size() == vertex_count() * 2 (or empty)
     *  - indices not empty
     *  - each submesh's index range fits within indices.size()
     *
     * @return true if the data appears consistent, false otherwise.
     */
    bool is_valid() const;
};
