#pragma once

#include <vector>
#include <cstdint>

#include "submesh.h"

/**
 * @brief Pure CPU-side mesh data.
 *
 * This structure contains raw attribute arrays and the index buffer.
 * It is format-agnostic (you can fill it from OBJ, GLTF, or any custom loader).
 *
 * No GPU handles live here. A MeshGroup later aggregates the data of many
 * MeshCPUData instances into a single VAO/VBO/EBO per chunk.
 */
struct MeshCPUData {
    // ---------------------------------------------------------------------
    // Vertex attributes
    // ---------------------------------------------------------------------

    /// Position buffer: 3 floats per vertex (x, y, z).
    std::vector<float> positions;

    /// Normal buffer: optional, 3 floats per vertex (nx, ny, nz).
    std::vector<float> normals;

    /// Texture coordinate buffer: optional, 2 floats per vertex (u, v).
    std::vector<float> texcoords;

    // ---------------------------------------------------------------------
    // Index buffer
    // ---------------------------------------------------------------------

    /// Triangle index buffer, referencing vertices by index.
    std::vector<std::uint32_t> indices;

    // ---------------------------------------------------------------------
    // Submeshes
    // ---------------------------------------------------------------------

    /**
     * @brief Logical submeshes which partition the index buffer.
     *
     * Each Submesh defines:
     *  - index_offset / index_count: a range in @ref indices
     *  - a Material pointer that should be used when rendering that range
     */
    std::vector<Submesh> submeshes;

    // ---------------------------------------------------------------------
    // Convenience queries
    // ---------------------------------------------------------------------

    /**
     * @brief Returns the number of vertices.
     *
     * Assumes positions are stored as tightly packed triples of floats.
     */
    std::size_t vertex_count() const { return positions.size() / 3; }

    /// Returns the number of indices.
    std::size_t index_count() const { return indices.size(); }

    /**
     * @brief Checks semantic correctness of the mesh data.
     *
     * Requirements:
     *  - positions.size() % 3 == 0
     *  - normals.size()   == vertex_count * 3 (or empty)
     *  - texcoords.size() == vertex_count * 2 (or empty)
     *  - indices not empty
     *  - each submesh's index range fits within indices.size()
     *
     * @return true if the data appears consistent, false otherwise.
     */
    bool is_valid() const;
};
