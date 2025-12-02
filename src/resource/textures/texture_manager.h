#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "texture.h"
#include "../resource_state.h"

/**
 * @brief Manager for deduplicating textures and driving their lifetime.
 *
 * Thread-safe via an internal mutex.
 */
class TextureManager {
    public:
    TextureManager() = default;

    std::shared_ptr<Texture> get(const std::string& path);

    void require(const std::string& path, ResourceState state);
    void release(const std::string& path, ResourceState state);

    void dump_state(int indent = 0) const;

    private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
    mutable std::mutex mtx_;
};
