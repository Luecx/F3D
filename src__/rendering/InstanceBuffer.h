#pragma once

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

#include "RenderScene.h"
#include "../gldata/ssbo_data.h"

class InstanceBuffer {
  public:
    InstanceBuffer();

    void sync(const RenderableList& renderables);
    void bind(GLuint binding_point);

    std::optional<std::size_t> base_instance(const Instances* component) const;
    std::size_t total_instances() const { return total_instances_; }

  private:
    struct Chunk {
        Instances* component{nullptr};
        std::size_t offset{0};
        std::size_t count{0};
    };

    void rebuild(const std::vector<Instances*>& components);
    void update_dirty_chunks();

    SSBOData buffer_;
    std::vector<Mat4f> staging_;
    std::vector<Chunk> chunks_;
    std::unordered_map<const Instances*, std::size_t> chunk_lookup_;
    std::size_t total_instances_{0};
    bool layout_dirty_{true};
};
