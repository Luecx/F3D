#include "texture.h"

#include <iostream>

#include "texture_loader.h"
#include "texture_manager.h"

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

void Texture::request_state(ResourceState state) {
    if (!manager_) {
        std::cerr << "Texture::request_state called without manager for '" << path_ << "'\n";
        return;
    }

    switch (state) {
    case ResourceState::Drive:
        // DRIVE is always satisfied once the Texture exists.
        return;

    case ResourceState::Ram:
        if (!has_ram_.load(std::memory_order_relaxed) &&
            !ram_load_pending_.exchange(true, std::memory_order_acq_rel)) {
            manager_->enqueue_ram_job(shared_from_this(), TextureJobType::LoadRam);
        }
        break;

    case ResourceState::Gpu:
        // Ensure RAM is (eventually) available.
        if (!has_ram_.load(std::memory_order_relaxed) &&
            !ram_load_pending_.exchange(true, std::memory_order_acq_rel)) {
            manager_->enqueue_ram_job(shared_from_this(), TextureJobType::LoadRam);
        }
        // Ensure GPU is (eventually) available.
        if (!has_gpu_.load(std::memory_order_relaxed) &&
            !gpu_load_pending_.exchange(true, std::memory_order_acq_rel)) {
            manager_->enqueue_gpu_job(shared_from_this(), TextureJobType::LoadGpu);
        }
        break;
    }
}

void Texture::release_state(ResourceState state) {
    if (!manager_) {
        std::cerr << "Texture::release_state called without manager for '" << path_ << "'\n";
        return;
    }

    switch (state) {
    case ResourceState::Drive:
        // Fully forgetting the texture (DRIVE=false) is typically a cache policy
        // handled by TextureManager; we do not implement eviction here.
        return;

    case ResourceState::Ram:
        if (has_ram_.load(std::memory_order_relaxed) &&
            !ram_unload_pending_.exchange(true, std::memory_order_acq_rel)) {
            manager_->enqueue_ram_job(shared_from_this(), TextureJobType::UnloadRam);
        }
        break;

    case ResourceState::Gpu:
        if (has_gpu_.load(std::memory_order_relaxed) &&
            !gpu_unload_pending_.exchange(true, std::memory_order_acq_rel)) {
            manager_->enqueue_gpu_job(shared_from_this(), TextureJobType::UnloadGpu);
        }
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
        std::cerr << "Texture::perform_load_ram: failed to load '" << path_ << "'\n";
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
        std::cerr << "Texture::perform_load_gpu: CPU data missing for '" << path_
                  << "'. Call request_state(RAM) first.\n";
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
