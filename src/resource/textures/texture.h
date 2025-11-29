#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "../resource_types.h"          // resources::ResourceState
#include "texture_cpu_data.h"
#include "texture_gpu_data.h"

class TextureManager;

/**
 * @brief High-level texture resource with async DRIVE/RAM/GPU states.
 *
 * A Texture instance represents a *logical* texture identified by a
 * filesystem path. It can hold:
 *
 *  - DRIVE: only the path is known, no data loaded.
 *  - RAM:   CPU pixel data is loaded (@ref TextureCPUData).
 *  - GPU:   GPU texture object is allocated (@ref TextureGPUData).
 *
 * RAM and GPU are *independent* flags, not a single enum state. You can:
 *
 *  - keep RAM=true, GPU=false (CPU-only),
 *  - keep RAM=false, GPU=true (GPU-only),
 *  - or both true/false.
 *
 * State transitions are performed asynchronously via a job queue owned
 * by @ref TextureManager. Calls to @ref request_state() and
 * @ref release_state() do not block; they simply enqueue work.
 */
class Texture : public std::enable_shared_from_this<Texture> {
  public:
    using Ptr = std::shared_ptr<Texture>;

    /**
     * @brief Returns the canonical path identifying this texture.
     */
    [[nodiscard]] const std::string& path() const noexcept { return path_; }

    /**
     * @brief Returns true if this texture currently satisfies the
     *        given state.
     *
     * Semantics:
     *  - DRIVE: true iff the texture object has a non-empty path.
     *  - RAM:   true iff CPU pixel data is currently loaded.
     *  - GPU:   true iff a GPU texture object is currently alive.
     */
    [[nodiscard]] bool is_in_state(resources::ResourceState state) const noexcept;

    /**
     * @brief Asynchronously requests that this texture *eventually*
     *        satisfies the given state.
     *
     * Examples:
     *  - request_state(DRIVE): no-op, DRIVE is always satisfied.
     *  - request_state(RAM):   will enqueue a CPU load job if RAM is missing.
     *  - request_state(GPU):   will enqueue both RAM and GPU jobs if needed.
     *
     * This function is thread-safe and non-blocking.
     */
    void request_state(resources::ResourceState state);

    /**
     * @brief Asynchronously requests that this texture *eventually*
     *        drops the given state.
     *
     * Examples:
     *  - release_state(RAM):   will enqueue a job that frees CPU pixels.
     *  - release_state(GPU):   will enqueue a job that destroys the GL texture.
     *
     * This function is thread-safe and non-blocking.
     */
    void release_state(resources::ResourceState state);

    /**
     * @brief Returns a pointer to the GPU texture wrapper, or nullptr if
     *        GPU is not available.
     *
     * This does not change any state; it only exposes the current pointer.
     * The caller must check @ref is_in_state(GPU) before using it.
     */
    [[nodiscard]] TextureGPUData* gpu_data() noexcept { return gpu_data_.get(); }

    /**
     * @brief Const overload of @ref gpu_data().
     */
    [[nodiscard]] const TextureGPUData* gpu_data() const noexcept { return gpu_data_.get(); }

    /**
     * @brief Returns a pointer to the CPU pixel data, or nullptr if RAM
     *        is not available.
     *
     * The caller must check @ref is_in_state(RAM) before dereferencing it.
     */
    [[nodiscard]] TextureCPUData* cpu_data() noexcept { return cpu_data_.get(); }

    /**
     * @brief Const overload of @ref cpu_data().
     */
    [[nodiscard]] const TextureCPUData* cpu_data() const noexcept { return cpu_data_.get(); }

  private:
    friend class TextureManager;

    /**
     * @brief Constructs a Texture with a given path and owning manager.
     *
     * This is private by design; only TextureManager should create
     * Texture instances so that deduplication by path works reliably.
     */
    Texture(std::string path, TextureManager* manager);

    // --- helper functions used by TextureManager's worker(s) ---

    /// Called on the loading thread: load CPU pixels (RAM state).
    void perform_load_ram();

    /// Called on the loading thread: free CPU pixels (RAM state).
    void perform_unload_ram();

    /// Called on the main/render thread: upload pixels to GPU.
    void perform_load_gpu();

    /// Called on the main/render thread: release GPU texture.
    void perform_unload_gpu();

  private:
    std::string path_;
    TextureManager* manager_ = nullptr;

    // CPU / GPU data (protected via atomic flags + manager's job ordering).
    std::shared_ptr<TextureCPUData> cpu_data_;
    std::shared_ptr<TextureGPUData> gpu_data_;

    // Atomic flags describing current availability.
    std::atomic<bool> has_ram_{false};
    std::atomic<bool> has_gpu_{false};

    // Pending flags to avoid double-queueing jobs.
    std::atomic<bool> ram_load_pending_{false};
    std::atomic<bool> ram_unload_pending_{false};
    std::atomic<bool> gpu_load_pending_{false};
    std::atomic<bool> gpu_unload_pending_{false};
};
