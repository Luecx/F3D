#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "mesh_cpu_data.h"
#include "../resource_state.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

class TextureManager;
class MaterialManager;

/**
 * @brief CPU-only mesh resource representing geometry on disk and in RAM.
 *
 * A Mesh supports the following conceptual states:
 *  - Drive: the mesh is known by its path, but no CPU data is loaded.
 *  - Ram:   CPU-side mesh data exists (@ref MeshCPUData).
 *
 * A Mesh does *not* own any GPU buffers. Aggregation of many Mesh objects
 * into a single VAO/VBO/EBO is handled by MeshGroupGPUData at the chunk
 * level. This keeps the per-mesh abstraction simple and streaming-friendly.
 */
class Mesh {
  public:
    using SPtr = std::shared_ptr<Mesh>;

    /**
     * @brief Construct a Mesh that corresponds to a file on disk.
     *
     * @param path Path to the mesh file (e.g. OBJ, GLTF).
     */
    explicit Mesh(std::string path);

    /// Mutable access to CPU data (may be nullptr if not in RAM).
    std::shared_ptr<MeshCPUData> cpu_data() { return cpu_data_; }

    /// Const access to CPU data (may be nullptr if not in RAM).
    std::shared_ptr<const MeshCPUData> cpu_data() const { return cpu_data_; }
    [[nodiscard]] const std::string& path() const { return path_; }

    // ------------------------------------------------------------------
    // Resource state API (Drive/Ram only – GPU is handled by MeshGroup).
    // ------------------------------------------------------------------

    /**
     * @brief Request that the mesh enters the given state.
     *
     * Semantics:
     *  - Drive: no-op (Drive existence is implicit via @ref path_).
     *  - Ram:   ensure CPU data is loaded from Drive.
     */
    void request(resources::ResourceState state);

    /**
     * @brief Release a previously requested state.
     *
     * Semantics:
     *  - Drive: no-op.
     *  - Ram:   free CPU data (drop @ref cpu_data_).
     *
     * Higher-level systems are responsible for coordinating reference
     * counting (e.g. multiple MeshGroup instances sharing a Mesh).
     */
    void release(resources::ResourceState state);

    /**
     * @brief Query whether the mesh is currently in the given state.
     *
     * Semantics:
     *  - Drive: always true.
     *  - Ram:   true iff CPU data is present.
     */
    bool is_in_state(resources::ResourceState state) const;
    int  ram_refcount() const { return ram_refcount_.load(std::memory_order_relaxed); }
    bool has_ram() const { return has_ram_; }

  private:
    /**
     * @brief Load CPU data from disk into @ref cpu_data_.
     *
     * This is where file parsing (OBJ, GLTF, etc.) should be implemented.
     * The default implementation creates an empty MeshCPUData, so it should
     * be replaced by a real loader in production.
     *
     * @return true on success, false on failure.
     */
    bool load_from_drive();

  private:
    std::string path_;
    std::shared_ptr<MeshCPUData> cpu_data_;
    std::atomic<int> ram_refcount_{0};
    bool has_ram_{false};
};
