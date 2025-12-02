#include "texture_manager.h"

#include "../../logging/logging.h"
#include "../resource_logging.h"

std::shared_ptr<Texture> TextureManager::get(const std::string& path) {
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = textures_.find(path);
    if (it != textures_.end()) {
        return it->second;
    }

    auto tex = std::make_shared<Texture>(path);
    textures_[path] = tex;
    return tex;
}

void TextureManager::require(const std::string& path, ResourceState state) {
    auto tex = get(path);
    if (tex) {
        tex->require(state);
    }
}

void TextureManager::release(const std::string& path, ResourceState state) {
    std::shared_ptr<Texture> tex;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = textures_.find(path);
        if (it != textures_.end()) {
            tex = it->second;
        }
    }

    if (tex) {
        tex->release(state);
    }
}

void TextureManager::dump_state(int indent) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string pad(indent, ' ');

    logging::log(reslog::TEXTURE, logging::INFO, pad + "Textures:");
    for (const auto& [path, tex] : textures_) {
        logging::log(
            reslog::TEXTURE,
            logging::INFO,
            pad + "  " + path +
                " cpu_users=" + std::to_string(tex->cpu_users()) +
                " gpu_users=" + std::to_string(tex->gpu_users())
        );
    }
}
