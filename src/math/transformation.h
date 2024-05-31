#ifndef F3D_TRANSFORMATION_H
#define F3D_TRANSFORMATION_H

#include "mat.h"
#include <algorithm>
#include <memory>
#include <vector>

class Transformation : public std::enable_shared_from_this<Transformation> {

    protected:
    Vec3f position;
    Vec3f rotation;
    Vec3f scale {1,1,1};

    Mat4f local_transformation;
    Mat4f global_transformation;

    std::shared_ptr<Transformation> parent;
    std::vector<std::shared_ptr<Transformation>> children;

    bool outdated = true;
    public:
    virtual ~Transformation() {
        if (parent) {
            remove_parent();
        }
        for (auto& child : children) {
            child->remove_parent();
        }
    }

    // setters to local fields in snake_case
    void set_position(const Vec3f& position) {
        set_outdated();
        this->position = position;
    }
    void set_rotation(const Vec3f& rotation) {
        set_outdated();
        this->rotation = rotation;
    }
    void set_scale(const Vec3f& scale) {
        set_outdated();
        this->scale = scale;
    }

    // local and global position
    Vec3f local_position() const {
        return position;
    }
    Vec3f local_xaxis() {
        this->update();
        return {local_transformation(0, 0), local_transformation(1, 0), local_transformation(2, 0)};
    }
    Vec3f local_yaxis() {
        this->update();
        return {local_transformation(0, 1), local_transformation(1, 1), local_transformation(2, 1)};
    }
    Vec3f local_zaxis() {
        this->update();
        return {local_transformation(0, 2), local_transformation(1, 2), local_transformation(2, 2)};
    }
    Vec3f global_position() {
        this->update();
        return {global_transformation(0, 3), global_transformation(1, 3), global_transformation(2, 3)};
    }
    Vec3f global_xaxis() {
        this->update();
        return {global_transformation(0, 0), global_transformation(1, 0), global_transformation(2, 0)};
    }
    Vec3f global_yaxis() {
        this->update();
        return {global_transformation(0, 1), global_transformation(1, 1), global_transformation(2, 1)};
    }
    Vec3f global_zaxis() {
        this->update();
        return {global_transformation(0, 2), global_transformation(1, 2), global_transformation(2, 2)};
    }

    void set_outdated() {
        if (!outdated) {
            outdated = true;
            for (auto& child : children) {
                child->set_outdated();
            }
        }
    }

    void update() {
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

    // managing parenting relationships
    bool remove_parent() {
        if (this->parent == nullptr) {
            return false;
        }
        // go through the parent's children and remove this child. consider problem with shared_ptr
        this->parent->children.erase(
            std::remove_if(this->parent->children.begin(), this->parent->children.end(), [this](const std::shared_ptr<Transformation>& child) { return child.get() == this; }),
            this->parent->children.end());

        this->parent = nullptr;
        set_outdated();
        return true;
    }

    bool set_parent(const std::shared_ptr<Transformation>& p_parent) {
        if (this->parent == p_parent || std::find_if(children.begin(), children.end(), [&p_parent](const std::shared_ptr<Transformation>& child) {
                                            return child.get() == p_parent.get();
                                        }) != children.end()) {
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

    public:
    bool add_child(const std::shared_ptr<Transformation>& child) {
        return child->set_parent(shared_from_this());
    }

    bool remove_child(const std::shared_ptr<Transformation>& child) {
        return child->remove_parent();
    }

    std::vector<std::shared_ptr<Transformation>> get_children() {
        return this->children;
    }
};

#endif    // F3D_TRANSFORMATION_H
