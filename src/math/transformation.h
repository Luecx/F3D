#ifndef F3D_TRANSFORMATION_H
#define F3D_TRANSFORMATION_H

#include "../ecs/include.h"
#include "mat.h"

#include <algorithm>
#include <memory>
#include <vector>

class Transformation : public ecs::ComponentInterface, public std::enable_shared_from_this<Transformation> {

    protected:
    Vec3f                        position {0, 0, 0};
    Vec3f                        rotation {0, 0, 0};
    Vec3f                        scale {1, 1, 1};

    Mat4f                        local_transformation {Mat4f::eye()};
    Mat4f                        global_transformation {Mat4f::eye()};

    std::shared_ptr<Transformation>              parent = nullptr;
    std::vector<std::shared_ptr<Transformation>> children;

    bool                         outdated = true;

    public:
    // construction
    Transformation(const Vec3f& position = {0, 0, 0}, const Vec3f& rotation = {0, 0, 0}, const Vec3f& scale = {1, 1, 1});

    virtual ~Transformation();

    // setters to local fields in snake_case
    void set_position(const Vec3f& position);
    void set_rotation(const Vec3f& rotation);
    void set_scale(const Vec3f& scale);

    // local and global position
    Vec3f local_position() const;
    Vec3f local_xaxis();
    Vec3f local_yaxis();
    Vec3f local_zaxis();
    Vec3f global_position();
    Vec3f global_xaxis();
    Vec3f global_yaxis();
    Vec3f global_zaxis();

    void set_outdated();

    void update();

    // managing parenting relationships
    bool remove_parent();

    bool set_parent(std::shared_ptr<Transformation> p_parent);

    public:
    bool add_child(std::shared_ptr<Transformation> child);

    bool remove_child(std::shared_ptr<Transformation> child);

    std::vector<std::shared_ptr<Transformation>> get_children();

    void component_removed() override {
        if (parent) {
            remove_parent();
        }
        for (auto& child : children) {
            child->remove_parent();
        }
    }
    void entity_activated() override {}
    void entity_deactivated() override {}
};

#endif    // F3D_TRANSFORMATION_H
