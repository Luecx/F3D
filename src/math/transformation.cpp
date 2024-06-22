#include "transformation.h"
#include <algorithm>
#include <memory>

Transformation::Transformation(const Vec3f& position, const Vec3f& rotation, const Vec3f& scale)
    : position(position)
    , rotation(rotation)
    , scale(scale) {}

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
    parent_transform->children.erase(std::remove(parent_transform->children.begin(),
                                                 parent_transform->children.end(), this->component_id.id),
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
    //auto& child_transform = ecs.get_entity(childID).get<Transformation>();
    //return child_transform.set_parent(this->get_entity_id());

    auto child_transform = (*ecs)[childID].get<Transformation>();
    return child_transform->set_parent(ecs::EntityID{this->component_id.id});
}

bool Transformation::remove_child(ecs::EntityID childID) {
    //auto& child_transform = ecs.get_entity(childID).get<Transformation>();
    //return child_transform.remove_parent();

    auto child_transform = (*ecs)[childID].get<Transformation>();
    return child_transform->remove_parent();
}

std::vector<ecs::EntityID> Transformation::get_children() {
    return this->children;
}
