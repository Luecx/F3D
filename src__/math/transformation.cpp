#include "transformation.h"

#include <algorithm>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <limits>
#include <memory>

Transformation::Transformation(const Vec3f& position, const Vec3f& rotation, const Vec3f& scale)
    : position(position), rotation(rotation), scale(scale) {}

Transformation::~Transformation() {
    if (parent.id != ecs::INVALID_ID) {
        remove_parent();
    }
    for (auto& child : children) {
        // This assumes ECS provides a way to access entities by ID and remove their parent
        (*ecs)[child].get<Transformation>()->remove_parent();
    }
}

void Transformation::set_position(const Vec3f& position) {
    set_outdated();
    this->position = position;
}

void Transformation::set_rotation(const Vec3f& rotation) {
    set_outdated();
    this->rotation = rotation;
}

void Transformation::set_scale(const Vec3f& scale) {
    set_outdated();
    this->scale = scale;
}

Vec3f Transformation::local_position() const { return position; }

Vec3f Transformation::local_rotation() const { return rotation; }

Vec3f Transformation::local_xaxis() {
    this->update();
    return {local_transformation(0, 0), local_transformation(1, 0), local_transformation(2, 0)};
}

Vec3f Transformation::local_yaxis() {
    this->update();
    return {local_transformation(0, 1), local_transformation(1, 1), local_transformation(2, 1)};
}

Vec3f Transformation::local_zaxis() {
    this->update();
    return {local_transformation(0, 2), local_transformation(1, 2), local_transformation(2, 2)};
}

Vec3f Transformation::global_position() const {
    const_cast<Transformation*>(this)->update();
    return {global_transformation(0, 3), global_transformation(1, 3), global_transformation(2, 3)};
}

Vec3f Transformation::global_xaxis() const {
    const_cast<Transformation*>(this)->update();
    return {global_transformation(0, 0), global_transformation(1, 0), global_transformation(2, 0)};
}

Vec3f Transformation::global_yaxis() const {
    const_cast<Transformation*>(this)->update();
    return {global_transformation(0, 1), global_transformation(1, 1), global_transformation(2, 1)};
}

Vec3f Transformation::global_zaxis() const {
    const_cast<Transformation*>(this)->update();
    return {global_transformation(0, 2), global_transformation(1, 2), global_transformation(2, 2)};
}

const Mat4f& Transformation::local_matrix() {
    this->update();
    return local_transformation;
}

const Mat4f& Transformation::global_matrix() {
    this->update();
    return global_transformation;
}

const Mat4f& Transformation::global_matrix() const {
    const_cast<Transformation*>(this)->update();
    return global_transformation;
}

namespace {
constexpr float kEpsilon = 1e-6f;
constexpr float kRadToDeg = 180.0f / static_cast<float>(M_PI);
} // namespace

Transformation Transformation::from_matrix(const Mat4f& matrix) {
    Transformation result;
    result.set_from_matrix(matrix);
    return result;
}

void Transformation::set_from_matrix(const Mat4f& matrix) {
    Vec3f translation{matrix(0, 3), matrix(1, 3), matrix(2, 3)};
    Vec3f column_x{matrix(0, 0), matrix(1, 0), matrix(2, 0)};
    Vec3f column_y{matrix(0, 1), matrix(1, 1), matrix(2, 1)};
    Vec3f column_z{matrix(0, 2), matrix(1, 2), matrix(2, 2)};

    float scale_x = column_x.length();
    float scale_y = column_y.length();
    float scale_z = column_z.length();

    Vec3f axes[3];
    axes[0] = (scale_x > kEpsilon) ? column_x * (1.0f / scale_x) : Vec3f{1.0f, 0.0f, 0.0f};
    axes[1] = (scale_y > kEpsilon) ? column_y * (1.0f / scale_y) : Vec3f{0.0f, 1.0f, 0.0f};
    axes[2] = (scale_z > kEpsilon) ? column_z * (1.0f / scale_z) : Vec3f{0.0f, 0.0f, 1.0f};

    Mat3f rotation_matrix;
    rotation_matrix(0, 0) = axes[0][0];
    rotation_matrix(1, 0) = axes[0][1];
    rotation_matrix(2, 0) = axes[0][2];
    rotation_matrix(0, 1) = axes[1][0];
    rotation_matrix(1, 1) = axes[1][1];
    rotation_matrix(2, 1) = axes[1][2];
    rotation_matrix(0, 2) = axes[2][0];
    rotation_matrix(1, 2) = axes[2][1];
    rotation_matrix(2, 2) = axes[2][2];

    float sy = std::sqrt(rotation_matrix(0, 0) * rotation_matrix(0, 0) + rotation_matrix(1, 0) * rotation_matrix(1, 0));
    bool singular = sy < kEpsilon;

    float rx = 0.0f;
    float ry = 0.0f;
    float rz = 0.0f;

    if (!singular) {
        rx = std::atan2(rotation_matrix(2, 1), rotation_matrix(2, 2));
        ry = std::atan2(-rotation_matrix(2, 0), sy);
        rz = std::atan2(rotation_matrix(1, 0), rotation_matrix(0, 0));
    } else {
        rx = std::atan2(-rotation_matrix(1, 2), rotation_matrix(1, 1));
        ry = std::atan2(-rotation_matrix(2, 0), sy);
        rz = 0.0f;
    }

    Vec3f euler_deg{rx * kRadToDeg, ry * kRadToDeg, rz * kRadToDeg};
    Vec3f new_scale{scale_x, scale_y, scale_z};

    set_scale(new_scale);
    set_rotation(euler_deg);
    set_position(translation);
}

void Transformation::set_look_at(const Vec3f& position, const Vec3f& target, const Vec3f& up) {
    Vec3f forward = target - position;
    if (forward.length() < kEpsilon) {
        forward = Vec3f{0.0f, 0.0f, -1.0f};
    }
    forward = forward.normalised();

    Vec3f up_dir = up;
    if (up_dir.length() < kEpsilon) {
        up_dir = Vec3f{0.0f, 1.0f, 0.0f};
    }
    up_dir = up_dir.normalised();

    Vec3f z_axis = -forward;
    Vec3f x_axis = up_dir.cross(z_axis);
    if (x_axis.length() < kEpsilon) {
        x_axis = Vec3f{1.0f, 0.0f, 0.0f};
    } else {
        x_axis = x_axis.normalised();
    }
    Vec3f y_axis = z_axis.cross(x_axis).normalised();

    Mat4f matrix = Mat4f::eye();
    matrix(0, 0) = x_axis[0];
    matrix(1, 0) = x_axis[1];
    matrix(2, 0) = x_axis[2];
    matrix(0, 1) = y_axis[0];
    matrix(1, 1) = y_axis[1];
    matrix(2, 1) = y_axis[2];
    matrix(0, 2) = z_axis[0];
    matrix(1, 2) = z_axis[1];
    matrix(2, 2) = z_axis[2];
    matrix(0, 3) = position[0];
    matrix(1, 3) = position[1];
    matrix(2, 3) = position[2];

    set_from_matrix(matrix);
}

Transformation Transformation::look_at(const Vec3f& position, const Vec3f& target, const Vec3f& up) {
    Transformation result;
    result.set_look_at(position, target, up);
    return result;
}

void Transformation::set_outdated() {
    if (!outdated) {
        outdated = true;
        for (auto& child : children) {
            (*ecs)[child].get<Transformation>()->set_outdated();
        }
    }
}

void Transformation::update() {
    if (outdated) {
        // local transformation
        local_transformation = Mat4f::eye();
        local_transformation.translate_3d(position);
        local_transformation.rotate_3d(rotation[2] * M_PI / 180, {0, 0, 1});
        local_transformation.rotate_3d(rotation[1] * M_PI / 180, {0, 1, 0});
        local_transformation.rotate_3d(rotation[0] * M_PI / 180, {1, 0, 0});
        local_transformation.scale_3d(scale);

        // global transformation
        if (parent.id != ecs::INVALID_ID) {
            auto* parent_transform = ecs->at(parent).get<Transformation>();
            parent_transform->update();
            global_transformation = parent_transform->global_transformation * local_transformation;
        } else {
            global_transformation = local_transformation;
        }

        outdated = false;
    }
}

bool Transformation::remove_parent() {
    if (parent.id == ecs::INVALID_ID) {
        return false;
    }

    auto* parent_transform = ecs->at(parent).get<Transformation>();
    parent_transform->children.erase(
        std::remove(parent_transform->children.begin(), parent_transform->children.end(), this->component_id.id),
        parent_transform->children.end());

    parent = ecs::EntityID{};
    set_outdated();
    return true;
}

bool Transformation::set_parent(ecs::EntityID parentID) {
    if (this->parent == parentID || std::find(children.begin(), children.end(), parentID) != children.end()) {
        return false;
    }
    if (this->parent.id != ecs::INVALID_ID) {
        remove_parent();
    }
    this->parent = parentID;
    auto parent_transform = (*ecs)[parentID].get<Transformation>();
    parent_transform->children.push_back(ecs::EntityID{this->component_id.id});
    set_outdated();
    return true;
}

bool Transformation::add_child(ecs::EntityID childID) {
    // auto& child_transform = ecs.get_entity(childID).get<Transformation>();
    // return child_transform.set_parent(this->get_entity_id());

    auto child_transform = (*ecs)[childID].get<Transformation>();
    return child_transform->set_parent(ecs::EntityID{this->component_id.id});
}

bool Transformation::remove_child(ecs::EntityID childID) {
    // auto& child_transform = ecs.get_entity(childID).get<Transformation>();
    // return child_transform.remove_parent();

    auto child_transform = (*ecs)[childID].get<Transformation>();
    return child_transform->remove_parent();
}

std::vector<ecs::EntityID> Transformation::get_children() { return this->children; }
