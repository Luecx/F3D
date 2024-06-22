#ifndef F3D_TRANSFORMATION_H
#define F3D_TRANSFORMATION_H

#include "mat.h"
#include <ecs.h>
#include <vector>

class ECS;

class Transformation : public ecs::ComponentOf<Transformation> {
    protected:
    Vec3f position {0, 0, 0};
    Vec3f rotation {0, 0, 0};
    Vec3f scale    {1, 1, 1};

    Mat4f local_transformation {Mat4f::eye()};
    Mat4f global_transformation {Mat4f::eye()};

    ecs::EntityID parent = ecs::EntityID{};
    std::vector<ecs::EntityID> children;

    bool outdated = true;

    public:
    // construction
    Transformation(const Vec3f& position = {0, 0, 0},
                   const Vec3f& rotation = {0, 0, 0},
                   const Vec3f& scale    = {1, 1, 1});

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
    bool set_parent(ecs::EntityID parentID);
    bool add_child(ecs::EntityID childID);
    bool remove_child(ecs::EntityID childID);
    std::vector<ecs::EntityID> get_children();



    // OVERRIDE --------------------------------------------------------------

    // when the component is removed from the entity
    void component_removed() override{
        if (parent.id != ecs::INVALID_ID) {
            remove_parent();
        }
        for (auto& child : children) {
            (*ecs)[child].get<Transformation>()->remove_parent();
        }
    };

    // when the entity is activated or deactivated
    void entity_activated() override{};
    void entity_deactivated() override{};

    // when another component is added or removed
    void other_component_added(ecs::Hash hash) override{};
    void other_component_removed(ecs::Hash hash) override{};

};

#endif // F3D_TRANSFORMATION_H
