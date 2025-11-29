#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#include "texture.h"

/**
 * @brief Job types for texture state transitions.
 *
 * RAM jobs are processed on the loading thread (no GL calls).
 * GPU jobs are processed on the main/render thread via
 * TextureManager::process_gpu_jobs().
 */
enum class TextureJobType {
    LoadRam,
    UnloadRam,
    LoadGpu,
    UnloadGpu
};

/**
 * @brief Represents a single texture job in the queue.
 */
struct TextureJob {
    std::weak_ptr<Texture> texture;
    TextureJobType type;
};

/**
 * @brief Manages loading, caching, and async state transitions for textures.
 *
 * TextureManager is responsible for:
 *  - Providing a unique Texture object per canonical path.
 *  - Running a background worker thread for RAM jobs (LoadRam/UnloadRam).
 *  - Holding a GPU job queue to be processed on the main thread.
 *
 * All operations are non-blocking from the caller's perspective.
 * The caller must periodically call @ref process_gpu_jobs() on the main
 * thread to execute GPU uploads and releases.
 */
class TextureManager {
  public:
    TextureManager();
    ~TextureManager();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    /**
     * @brief Returns a shared Texture handle for the given path.
     *
     * If the texture has been requested before, the existing instance
     * is returned. Otherwise, a new Texture is created and stored.
     *
     * Calling this function does *not* automatically load or upload
     * any data; you must call request_state(RAM / GPU) on the texture
     * to schedule work.
     */
    std::shared_ptr<Texture> get(const std::string& path);

    /**
     * @brief Enqueues a RAM-related job (LoadRam / UnloadRam) for the
     *        background loading thread.
     *
     * This is typically called by Texture::request_state() /
     * Texture::release_state() and should not be used directly.
     */
    void enqueue_ram_job(const std::shared_ptr<Texture>& tex, TextureJobType type);

    /**
     * @brief Enqueues a GPU-related job (LoadGpu / UnloadGpu) that must
     *        be executed on the main/render thread.
     *
     * You must periodically call @ref process_gpu_jobs() from the main
     * thread to process these jobs.
     */
    void enqueue_gpu_job(const std::shared_ptr<Texture>& tex, TextureJobType type);

    /**
     * @brief Processes all pending GPU jobs.
     *
     * This function must be called on the main/render thread that owns
     * the OpenGL context. It executes LoadGpu/UnloadGpu operations by
     * calling Texture::perform_load_gpu() / perform_unload_gpu() for
     * each queued job.
     */
    void process_gpu_jobs();

  private:
    // Worker thread loop for RAM jobs.
    void ram_worker_loop();

    // Maps canonical path -> Texture resource.
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
    std::mutex textures_mutex_;

    // RAM job queue and worker.
    std::queue<TextureJob> ram_jobs_;
    std::mutex ram_mutex_;
    std::condition_variable ram_cv_;
    std::thread ram_worker_;
    bool ram_stop_ = false;

    // GPU job queue (processed on main thread).
    std::queue<TextureJob> gpu_jobs_;
    std::mutex gpu_mutex_;
};
