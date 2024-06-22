#include "math/mat.h"
#include "math/transformation.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

int main() {


    // create an ecs, create two entities, assign a transformation and set the first transformation to be the parent of the second
    ecs::ECS ecs;
    auto entity1 = ecs.spawn();
    auto entity2 = ecs.spawn();

    auto comp1 = ecs[entity1].assign<Transformation>();
    auto comp2 = ecs[entity2].assign<Transformation>();

    ecs[entity2].get<Transformation>()->set_parent(entity1);

    // display both transformations
    std::cout << "Entity 1: " << std::endl;
    std::cout << "Position: " << ecs[entity1].get<Transformation>() << std::endl;
    std::cout << "Local Position: " << ecs[entity1].get<Transformation>()->local_position() << std::endl;
    std::cout << "Global Position: " << ecs[entity1].get<Transformation>()->global_position() << std::endl;
    std::cout << std::endl;

    // randomly access it and change elements and time that entire thing

    // timer in nano seconds
    auto now = []() { return std::chrono::high_resolution_clock::now(); };
    auto time = now();

    auto trans1 = ecs[entity1].get<Transformation>();
    auto trans2 = ecs[entity2].get<Transformation>();
    for (float i = 0; i < 1e6; i ++) {
        ecs[entity1].get<Transformation>()->set_position({i, i, i});
        ecs[entity1].get<Transformation>()->set_rotation({i, i, i});
        ecs[entity1].get<Transformation>()->set_scale({i, i, i});

        ecs[entity2].get<Transformation>()->set_position({i, i, i});
        ecs[entity2].get<Transformation>()->set_rotation({i, i, i});
        ecs[entity2].get<Transformation>()->set_scale({i, i, i});
        ecs[entity2].get<Transformation>()->update();
    }

    // display elapsed
    auto time2 = now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time).count();
    std::cout << elapsed / 1e6 << "ns / sample" << std::endl;
    std::cout << "could do this n-times per millisecond: " << 1e6 / (elapsed / 1e6) << std::endl;

    // same measurement but using trans1 and trans2
    time = now();
    for (float i = 0; i < 1e6; i ++) {
        trans1->set_position({i, i, i});
        trans1->set_rotation({i, i, i});
        trans1->set_scale({i, i, i});

        trans2->set_position({i, i, i});
        trans2->set_rotation({i, i, i});
        trans2->set_scale({i, i, i});

       //  trans2->update();
    }
    time2 = now();
    elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time).count();
    std::cout << elapsed / 1e6 << "ns / sample" << std::endl;
    std::cout << "could do this n-times per millisecond: " << std::fixed << 1e6 / (elapsed / 1e6) << std::endl;


    return 0;
}
