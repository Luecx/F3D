#include "texture.h"

#include "texture_loader.h"
#include "texture_manager.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

using resources::ResourceState;

Texture::Texture(std::string path, TextureManager* manager)
    : path_(std::move(path)), manager_(manager) {}

bool Texture::is_in_state(ResourceState state) const noexcept {
    switch (state) {
    case ResourceState::Drive:
        return !path_.empty();
    case ResourceState::Ram:
        return has_ram_.load(std::memory_order_relaxed);
    case ResourceState::Gpu:
        return has_gpu_.load(std::memory_order_relaxed);
    }
    return false;
}

void Texture::request(ResourceState state) {
    if (!manager_) {
        logging::log(reslog::TEXTURE, logging::ERROR, "Texture::request called without manager for '" + path_ + "'");
        return;
    }

    const std::string state_str = std::string(resources::to_string(state));
    logging::log(reslog::TEXTURE, logging::INFO, "Requesting texture '" + path_ + "' to be in " + state_str);

    switch (state) {
    case ResourceState::Drive:
        logging::log(reslog::TEXTURE, logging::DEBUG, "Drive state implicit for '" + path_ + "'");
        return;

    case ResourceState::Ram:
        {
            const int prev = ram_refcount_.fetch_add(1, std::memory_order_acq_rel);
            if (prev == 0 && !has_ram_.load(std::memory_order_relaxed) &&
                !ram_load_pending_.exchange(true, std::memory_order_acq_rel)) {
                manager_->enqueue_ram_job(shared_from_this(), TextureJobType::LoadRam);
                logging::log(reslog::TEXTURE, logging::INFO, "Enqueued RAM load for '" + path_ + "'");
            } else {
                logging::log(reslog::TEXTURE, logging::DEBUG, "RAM already requested/present for '" + path_ + "'");
            }
        }
        break;

    case ResourceState::Gpu:
        // GPU implicitly requires RAM.
        request(ResourceState::Ram);
        {
            const int prev = gpu_refcount_.fetch_add(1, std::memory_order_acq_rel);
            if (prev == 0 && !has_gpu_.load(std::memory_order_relaxed) &&
                !gpu_load_pending_.exchange(true, std::memory_order_acq_rel)) {
                manager_->enqueue_gpu_job(shared_from_this(), TextureJobType::LoadGpu);
                logging::log(reslog::TEXTURE, logging::INFO, "Enqueued GPU load for '" + path_ + "'");
            } else {
                logging::log(reslog::TEXTURE, logging::DEBUG, "GPU already requested/present for '" + path_ + "'");
            }
        }
        break;
    }
}

void Texture::release(ResourceState state) {
    if (!manager_) {
        logging::log(reslog::TEXTURE, logging::ERROR, "Texture::release called without manager for '" + path_ + "'");
        return;
    }

    const std::string state_str = std::string(resources::to_string(state));
    logging::log(reslog::TEXTURE, logging::INFO, "Releasing texture '" + path_ + "' from " + state_str);

    switch (state) {
    case ResourceState::Drive:
        // Fully forgetting the texture (DRIVE=false) is typically a cache policy
        // handled by TextureManager; we do not implement eviction here.
        return;

    case ResourceState::Ram:
        {
            const int prev = ram_refcount_.fetch_sub(1, std::memory_order_acq_rel);
            if (prev <= 0) {
                ram_refcount_.store(0, std::memory_order_relaxed);
                return;
            }
            if (prev == 1 && has_ram_.load(std::memory_order_relaxed) &&
                !ram_unload_pending_.exchange(true, std::memory_order_acq_rel)) {
                manager_->enqueue_ram_job(shared_from_this(), TextureJobType::UnloadRam);
            }
        }
        break;

    case ResourceState::Gpu:
        {
            const int prev = gpu_refcount_.fetch_sub(1, std::memory_order_acq_rel);
            if (prev <= 0) {
                gpu_refcount_.store(0, std::memory_order_relaxed);
                // Still release the implicit RAM reference we took for GPU.
                release(ResourceState::Ram);
                return;
            }
            if (prev == 1 && has_gpu_.load(std::memory_order_relaxed) &&
                !gpu_unload_pending_.exchange(true, std::memory_order_acq_rel)) {
                manager_->enqueue_gpu_job(shared_from_this(), TextureJobType::UnloadGpu);
            }
        }
        // Drop the implicit RAM reference acquired alongside the GPU request.
        release(ResourceState::Ram);
        break;
    }
}

// --- RAM side (loading thread) ---

void Texture::perform_load_ram() {
    // If RAM is already there, nothing to do.
    if (has_ram_.load(std::memory_order_relaxed)) {
        ram_load_pending_.store(false, std::memory_order_relaxed);
        return;
    }

    TextureCPUData data = TextureLoader::load_from_file(path_);
    if (!data.valid()) {
        logging::log(reslog::TEXTURE, logging::ERROR, "Texture::perform_load_ram: failed to load '" + path_ + "'");
        ram_load_pending_.store(false, std::memory_order_relaxed);
        return;
    }

    cpu_data_ = std::make_shared<TextureCPUData>(std::move(data));
    has_ram_.store(true, std::memory_order_release);
    ram_load_pending_.store(false, std::memory_order_relaxed);
}

void Texture::perform_unload_ram() {
    cpu_data_.reset();
    has_ram_.store(false, std::memory_order_release);
    ram_unload_pending_.store(false, std::memory_order_relaxed);
}

// --- GPU side (main / render thread) ---

void Texture::perform_load_gpu() {
    if (has_gpu_.load(std::memory_order_relaxed)) {
        gpu_load_pending_.store(false, std::memory_order_relaxed);
        return;
    }

    if (!cpu_data_ || !cpu_data_->valid()) {
        logging::log(reslog::TEXTURE, logging::ERROR, "Texture::perform_load_gpu: CPU data missing for '" + path_ +
                     "'. Call request(RAM) first.");
        gpu_load_pending_.store(false, std::memory_order_relaxed);
        return;
    }

    if (!gpu_data_) {
        gpu_data_ = std::make_shared<TextureGPUData>();
    }

    gpu_data_->upload_from_cpu(*cpu_data_);
    has_gpu_.store(true, std::memory_order_release);
    gpu_load_pending_.store(false, std::memory_order_relaxed);
}

void Texture::perform_unload_gpu() {
    if (gpu_data_) {
        gpu_data_->release();
        gpu_data_.reset();
    }
    has_gpu_.store(false, std::memory_order_release);
    gpu_unload_pending_.store(false, std::memory_order_relaxed);
}
