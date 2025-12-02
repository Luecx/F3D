#pragma once

#include <memory>
#include <string>

#include "../resource_base.h"
#include "../resource_state.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

#include "texture_cpu_data.h"
#include "texture_gpu_data.h"
#include "texture_loader.h"

/**
 * @brief Unified texture resource with CPU/GPU data managed via ResourceBase.
 *
 * Semantics:
 *  - require(Cpu):   ensure CPU pixels loaded (synchronous for now).
 *  - require(Gpu):   ensure GPU texture uploaded; may call require(Cpu) inside
 *                    impl_load(Gpu) if CPU is needed for upload.
 *  - release(Cpu):   when cpu_users==0, drop CPU pixels (policy).
 *  - release(Gpu):   when gpu_users==0, destroy GPU texture object.
 */
class Texture : public ResourceBase, public std::enable_shared_from_this<Texture> {
    public:
    using Ptr = std::shared_ptr<Texture>;

    explicit Texture(std::string path)
        : path_(std::move(path)) {}

    const std::string& path() const noexcept { return path_; }

    TextureCPUData*       cpu_data()       noexcept { return cpu_data_.get(); }
    const TextureCPUData* cpu_data() const noexcept { return cpu_data_.get(); }

    TextureGPUData*       gpu_data()       noexcept { return gpu_data_.get(); }
    const TextureGPUData* gpu_data() const noexcept { return gpu_data_.get(); }

    protected:
    void impl_load(ResourceState state) override {
        switch (state) {
            case ResourceState::Cpu:
                load_cpu();
                break;
            case ResourceState::Gpu:
                load_gpu();
                break;
            case ResourceState::Drive:
            default:
                // Drive has no managed load/unload in this design.
                break;
        }
    }

    void impl_unload(ResourceState state) override {
        switch (state) {
            case ResourceState::Cpu:
                unload_cpu();
                break;
            case ResourceState::Gpu:
                unload_gpu();
                break;
            case ResourceState::Drive:
            default:
                // no-op
                break;
        }
    }

    private:
    void load_cpu() {
        if (cpu_data_ && cpu_data_->valid()) {
            return;
        }

        cpu_data_ = std::make_unique<TextureCPUData>(TextureLoader::load_from_file(path_));

        if (!cpu_data_->valid()) {
            logging::log(reslog::TEXTURE, logging::ERROR,
                         "Failed to load texture '" + path_ + "' to CPU");
        } else {
            logging::log(reslog::TEXTURE, logging::INFO,
                         "Loaded texture to CPU '" + path_ + "'");
        }
    }

    void unload_cpu() {
        if (!cpu_data_) {
            return;
        }
        cpu_data_->reset();
        cpu_data_.reset();
        logging::log(reslog::TEXTURE, logging::INFO,
                     "Released CPU data for '" + path_ + "'");
    }

    void load_gpu() {
        if (!gpu_data_) {
            gpu_data_ = std::make_unique<TextureGPUData>();
        }

        // Ensure CPU data exists. We could also just fail if cpu_users()==0,
        // but for now we load lazily here.
        if (!cpu_data_ || !cpu_data_->valid()) {
            load_cpu();
        }

        if (cpu_data_ && cpu_data_->valid()) {
            gpu_data_->upload_from_cpu(*cpu_data_);
            logging::log(reslog::TEXTURE, logging::INFO,
                         "Uploaded texture to GPU '" + path_ + "'");

            // Optional policy: if no CPU users, drop CPU data after upload.
            if (cpu_users() == 0) {
                unload_cpu();
                logging::log(reslog::TEXTURE, logging::DEBUG,
                             "Dropped CPU data after GPU upload for '" + path_ + "'");
            }
        } else {
            logging::log(reslog::TEXTURE, logging::ERROR,
                         "Texture '" + path_ + "' has no valid CPU data for GPU upload");
        }
    }

    void unload_gpu() {
        if (!gpu_data_) {
            return;
        }
        gpu_data_->release();
        gpu_data_.reset();
        logging::log(reslog::TEXTURE, logging::INFO,
                     "Released GPU data for '" + path_ + "'");
    }

    std::string path_;
    std::unique_ptr<TextureCPUData> cpu_data_;
    std::unique_ptr<TextureGPUData> gpu_data_;
};
