#include "InstanceBuffer.h"

#include <algorithm>
#include <cstring>
#include <cstdint>

#include "../logging/logging.h"

namespace {
constexpr std::size_t kMat4SizeBytes = sizeof(Mat4f);
}

InstanceBuffer::InstanceBuffer() = default;

void InstanceBuffer::sync(const RenderableList& renderables) {
    logging::log(0,
                 logging::DEBUG,
                 "InstanceBuffer::sync begin for " + std::to_string(renderables.size()) + " renderables");

    std::vector<Instances*> components;
    components.reserve(renderables.size());
    for (const auto& renderable : renderables) {
        auto* instances = renderable.instances;
        if (!instances || instances->count() == 0) {
            continue;
        }
        components.push_back(instances);
    }

    std::sort(components.begin(), components.end());
    components.erase(std::unique(components.begin(), components.end()), components.end());

    bool needs_rebuild = layout_dirty_ || components.size() != chunks_.size();
    if (!needs_rebuild) {
        for (std::size_t i = 0; i < components.size(); ++i) {
            if (chunks_[i].component != components[i]) {
                needs_rebuild = true;
                break;
            }
            if (components[i]->structure_dirty() || chunks_[i].count != components[i]->count()) {
                needs_rebuild = true;
                break;
            }
        }
    }

    if (needs_rebuild) {
        logging::log(0,
                     logging::DEBUG,
                     "InstanceBuffer::sync rebuilding layout with " + std::to_string(components.size())
                         + " components");
        rebuild(components);
    } else {
        update_dirty_chunks();
        if (layout_dirty_) {
            logging::log(0,
                         logging::DEBUG,
                         "InstanceBuffer::sync detected structure change during dirty update; rebuilding");
            rebuild(components);
        }
    }
}

void InstanceBuffer::bind(GLuint binding_point) {
    buffer_.bind(binding_point);
}

std::optional<std::size_t> InstanceBuffer::base_instance(const Instances* component) const {
    if (!component) {
        return std::nullopt;
    }
    auto it = chunk_lookup_.find(component);
    if (it == chunk_lookup_.end()) {
        logging::log(0,
                     logging::DEBUG,
                     "InstanceBuffer::base_instance missing chunk for component "
                         + std::to_string(reinterpret_cast<std::uintptr_t>(component)));
        return std::nullopt;
    }
    const auto& chunk = chunks_[it->second];
    return chunk.offset;
}

void InstanceBuffer::rebuild(const std::vector<Instances*>& components) {
    chunks_.clear();
    chunk_lookup_.clear();
    staging_.clear();
    total_instances_ = 0;

    chunks_.reserve(components.size());
    for (auto* component : components) {
        if (!component) {
            continue;
        }
        std::size_t count = component->count();
        if (count == 0) {
            continue;
        }
        Chunk chunk;
        chunk.component = component;
        chunk.offset = total_instances_;
        chunk.count = count;
        logging::log(0,
                     logging::DEBUG,
                     "InstanceBuffer::rebuild chunk component="
                         + std::to_string(reinterpret_cast<std::uintptr_t>(component)) + " offset="
                         + std::to_string(chunk.offset) + " count=" + std::to_string(chunk.count));
        chunks_.push_back(chunk);
        chunk_lookup_[component] = chunks_.size() - 1;
        total_instances_ += count;
    }

    staging_.resize(total_instances_);
    for (auto& chunk : chunks_) {
        if (chunk.count == 0 || !chunk.component) {
            continue;
        }
        std::memcpy(staging_.data() + chunk.offset,
                    chunk.component->data(),
                    chunk.count * kMat4SizeBytes);
        chunk.component->clear_dirty_flags();
    }

    GLsizeiptr byte_size = static_cast<GLsizeiptr>(staging_.size() * kMat4SizeBytes);
    logging::log(0,
                 logging::DEBUG,
                 "InstanceBuffer::rebuild uploading " + std::to_string(staging_.size())
                     + " matrices (" + std::to_string(byte_size) + " bytes)");
    if (byte_size > 0) {
        buffer_.update_data(byte_size, staging_.data(), static_cast<GLenum>(GL_DYNAMIC_DRAW));
    } else {
        buffer_.update_data(static_cast<GLsizeiptr>(sizeof(Mat4f)), nullptr, static_cast<GLenum>(GL_DYNAMIC_DRAW));
    }

    layout_dirty_ = false;
}

void InstanceBuffer::update_dirty_chunks() {
    for (auto& chunk : chunks_) {
        auto* component = chunk.component;
        if (!component || !component->dirty()) {
            continue;
        }
        if (component->structure_dirty() || component->count() != chunk.count) {
            layout_dirty_ = true;
            break;
        }
        if (chunk.count == 0) {
            component->clear_dirty_flags();
            continue;
        }

        logging::log(0,
                     logging::DEBUG,
                     "InstanceBuffer::update_dirty chunk offset=" + std::to_string(chunk.offset)
                         + " count=" + std::to_string(chunk.count));
        std::memcpy(staging_.data() + chunk.offset,
                    component->data(),
                    chunk.count * kMat4SizeBytes);

        const auto byte_offset = static_cast<GLintptr>(chunk.offset * kMat4SizeBytes);
        const auto byte_size = static_cast<GLsizeiptr>(chunk.count * kMat4SizeBytes);
        buffer_.update_data(byte_size, component->data(), byte_offset, GL_DYNAMIC_DRAW);
        component->clear_dirty_flags();
    }

    // Keep staging copy coherent for CPU introspection when no rebuild occurs.
}
