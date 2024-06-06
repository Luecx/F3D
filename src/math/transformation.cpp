//
// Created by finn on 5/31/24.
//

#include "transformation.h"
Transformation::Transformation(const Vec3f& position, const Vec3f& rotation, const Vec3f& scale)
    : position(position)
    , rotation(rotation)
    , scale(scale) {}
Transformation::~Transformation() {
    if (parent) {
        remove_parent();
    }
    for (auto& child : children) {
        child->remove_parent();
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
Vec3f Transformation::local_position() const {
    return position;
}
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
Vec3f Transformation::global_position() {
    this->update();
    return {global_transformation(0, 3), global_transformation(1, 3), global_transformation(2, 3)};
}
Vec3f Transformation::global_xaxis() {
    this->update();
    return {global_transformation(0, 0), global_transformation(1, 0), global_transformation(2, 0)};
}
Vec3f Transformation::global_yaxis() {
    this->update();
    return {global_transformation(0, 1), global_transformation(1, 1), global_transformation(2, 1)};
}
Vec3f Transformation::global_zaxis() {
    this->update();
    return {global_transformation(0, 2), global_transformation(1, 2), global_transformation(2, 2)};
}
void Transformation::set_outdated() {
    if (!outdated) {
        outdated = true;
        for (auto& child : children) {
            child->set_outdated();
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
        if (parent) {
            parent->update();
            global_transformation = parent->global_transformation.matmul(local_transformation);
        } else {
            global_transformation = local_transformation;
        }

        outdated = false;
    }
}
bool Transformation::remove_parent() {
    if (this->parent == nullptr) {
        return false;
    }
    // go through the parent's children and remove this child. consider problem with shared_ptr
    this->parent->children.erase(std::remove_if(this->parent->children.begin(), this->parent->children.end(), [this](std::shared_ptr<Transformation> child) { return child.get() == this; }),
                                 this->parent->children.end());

    this->parent = nullptr;
    set_outdated();
    return true;
}
bool Transformation::set_parent(std::shared_ptr<Transformation> p_parent) {
    if (this->parent == p_parent || std::find_if(children.begin(), children.end(), [&p_parent](const std::shared_ptr<Transformation> child) { return child == p_parent; }) != children.end()) {
        return false;
    }
    if (this->parent) {
        remove_parent();
    }
    this->parent = p_parent;
    this->parent->children.push_back(this->shared_from_this());
    set_outdated();
    return true;
}
bool Transformation::add_child(std::shared_ptr<Transformation> child) {
    return child->set_parent(this->shared_from_this());
}
bool Transformation::remove_child(std::shared_ptr<Transformation> child) {
    return child->remove_parent();
}
std::vector<std::shared_ptr<Transformation>> Transformation::get_children() {
    return this->children;
}
