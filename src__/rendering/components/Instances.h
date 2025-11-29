#pragma once

#include <ecs.h>

#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../math/mat.h"

struct Instances : ecs::ComponentOf<Instances> {
    Instances() = default;

    Mat4f& add(const Mat4f& transform) {
        transforms_.push_back(transform);
        mark_dirty(true);
        return transforms_.back();
    }

    Mat4f& add(const Vec3f& position, const Vec3f& rotation = Vec3f{0.0f, 0.0f, 0.0f},
               const Vec3f& scale = Vec3f{1.0f, 1.0f, 1.0f}) {
        return add(compose_transform(position, rotation, scale));
    }

    void set(std::size_t index, const Mat4f& transform) {
        if (index >= transforms_.size()) {
            return;
        }
        transforms_[index] = transform;
        mark_dirty(false);
    }

    void set(std::size_t index, const Vec3f& position, const Vec3f& rotation = Vec3f{0.0f, 0.0f, 0.0f},
             const Vec3f& scale = Vec3f{1.0f, 1.0f, 1.0f}) {
        set(index, compose_transform(position, rotation, scale));
    }

    Mat4f& edit(std::size_t index) {
        mark_dirty(false);
        return transforms_.at(index);
    }

    void clear() {
        transforms_.clear();
        mark_dirty(true);
    }

    [[nodiscard]] std::size_t count() const { return transforms_.size(); }
    [[nodiscard]] bool empty() const { return transforms_.empty(); }
    [[nodiscard]] const Mat4f* data() const { return transforms_.data(); }
    [[nodiscard]] const std::vector<Mat4f>& transforms() const { return transforms_; }
    [[nodiscard]] bool dirty() const { return dirty_; }
    [[nodiscard]] bool structure_dirty() const { return structure_dirty_; }

    void clear_dirty_flags() {
        dirty_ = false;
        structure_dirty_ = false;
    }

  private:
    static Mat4f compose_transform(const Vec3f& position, const Vec3f& rotation, const Vec3f& scale) {
        Mat4f matrix = Mat4f::eye();
        matrix.translate_3d(position);
        constexpr float deg_to_rad = 3.14159265358979323846f / 180.0f;
        matrix.rotate_3d(rotation[2] * deg_to_rad, Vec3f{0.0f, 0.0f, 1.0f});
        matrix.rotate_3d(rotation[1] * deg_to_rad, Vec3f{0.0f, 1.0f, 0.0f});
        matrix.rotate_3d(rotation[0] * deg_to_rad, Vec3f{1.0f, 0.0f, 0.0f});
        matrix.scale_3d(scale);
        return matrix;
    }

    void mark_dirty(bool structure_change) {
        dirty_ = true;
        if (structure_change) {
            structure_dirty_ = true;
        }
    }

    std::vector<Mat4f> transforms_;
    bool dirty_{false};
    bool structure_dirty_{false};
};

static_assert(std::is_trivially_copyable_v<Mat4f>, "Instance matrices must be trivially copyable");
