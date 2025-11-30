#include "texture_manager.h"

#include <utility>
#include "../resource_logging.h"
#include "../../logging/logging.h"

TextureManager::TextureManager() = default;
TextureManager::~TextureManager() = default;

std::shared_ptr<Texture> TextureManager::get(const std::string& path) {
    std::lock_guard<std::mutex> lock(textures_mutex_);
    auto it = textures_.find(path);
    if (it != textures_.end()) {
        return it->second;
    }
    // Texture constructor is private; construct via new and shared_ptr.
    auto tex = std::shared_ptr<Texture>(new Texture(path, this));
    textures_[path] = tex;
    return tex;
}

void TextureManager::enqueue_ram_job(const std::shared_ptr<Texture>& tex, TextureJobType type) {
    if (!tex) return;
    switch (type) {
    case TextureJobType::LoadRam:
        tex->perform_load_ram();
        break;
    case TextureJobType::UnloadRam:
        tex->perform_unload_ram();
        break;
    default:
        break;
    }
}

void TextureManager::enqueue_gpu_job(const std::shared_ptr<Texture>& tex, TextureJobType type) {
    std::lock_guard<std::mutex> lock(gpu_mutex_);
    gpu_jobs_.push(TextureJob{tex, type});
}

void TextureManager::process_gpu_jobs() {
    std::queue<TextureJob> local;
    {
        std::lock_guard<std::mutex> lock(gpu_mutex_);
        std::swap(local, gpu_jobs_);
    }

    while (!local.empty()) {
        TextureJob job = std::move(local.front());
        local.pop();

        if (auto tex = job.texture.lock()) {
            switch (job.type) {
            case TextureJobType::LoadGpu:
                tex->perform_load_gpu();
                break;
            case TextureJobType::UnloadGpu:
                tex->perform_unload_gpu();
                break;
            case TextureJobType::LoadRam:
            case TextureJobType::UnloadRam:
                // These should not be scheduled to the GPU queue.
                break;
            }
        }
    }
}

void TextureManager::dump_state(int indent) const {
    std::lock_guard<std::mutex> lock(textures_mutex_);
    const std::string pad(indent, ' ');
    logging::log(reslog::TEXTURE, logging::INFO, pad + "TextureManager state:");
    for (const auto& [path, tex] : textures_) {
        if (!tex) continue;
        logging::log(reslog::TEXTURE, logging::INFO,
                     pad + "  " + path +
                         " RAM=" + (tex->has_ram() ? "yes" : "no") +
                         " GPU=" + (tex->has_gpu() ? "yes" : "no") +
                         " rc_ram=" + std::to_string(tex->ram_refcount()) +
                         " rc_gpu=" + std::to_string(tex->gpu_refcount()));
    }
}
