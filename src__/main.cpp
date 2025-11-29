// #include <array>
//
// #include <ecs.h>
//
// #include "logging/logging.h"
// #include "rendering/MasterRenderer.h"
// #include "rendering/components/Instances.h"
// #include "rendering/components/ModelComponent.h"
// #include "rendering/components/ShadowCaster.h"
// #include "rendering/components/Transparency.h"
// #include "rendering/components/Visible.h"
// #include "camera/camera_controller.h"
// #include "camera/perspective_camera.h"
// #include "lighting/point_light.h"
// #include "math/transformation.h"
// #include "resources/resource_types.h"
//
// int main() {
//     logging::set_logger(logging::Logger()
//                             .channel(0)
//                             .file_output("log.txt", logging::ALL)
//                             .cout(logging::ALL)
//                             .timestamp()
//                             .channel(1)
//                             .file_output("log_resources.txt", logging::ALL)
//                             .file_output("log.txt", logging::ALL)
//                             .cout(logging::ALL)
//                             .timestamp());
//
//     logging::log(0, logging::INFO, "Starting F3D main renderer setup");
//     MasterRenderer renderer;
//     constexpr int window_width = 800;
//     constexpr int window_height = 600;
//     if (!renderer.initialize(window_width, window_height, "F3D")) {
//         logging::log(0, logging::ERROR, "Renderer initialization failed.");
//         return -1;
//     }
//
//     logging::log(0, logging::INFO, "Renderer initialized. Acquiring resources");
//     auto& resource_manager = renderer.resources();
//     auto& ecs_system = renderer.ecs();
//
//     auto mesh = resource_manager.get_mesh("../resources/models/cube.obj");
//     if (!mesh) {
//         logging::log(1, logging::ERROR, "Failed to acquire mesh resource.");
//         return -1;
//     }
//     mesh->load(resources::ResourceState::Ram);
//     mesh->load(resources::ResourceState::Gpu);
//
//     auto floor_entity_id = ecs_system.spawn(true);
//     auto& floor_entity = ecs_system[floor_entity_id];
//     floor_entity.assign<ModelComponent>(mesh);
//     floor_entity.assign<Visibility>(true);
//     floor_entity.assign<ShadowCaster>(true);
//     floor_entity.assign<Instances>();
//     if (auto* instances = floor_entity.get<Instances>()) {
//         instances->clear();
//         instances->add(Vec3f{0.0f, -1.5f, 0.0f}, Vec3f{0.0f, 0.0f, 0.0f}, Vec3f{8.0f, 0.25f, 8.0f});
//     }
//
//     constexpr float kGroundTop = -1.5f + 0.25f * 0.5f;
//     struct BlockConfig {
//         Vec3f position;
//         Vec3f scale;
//     };
//
//     std::array<BlockConfig, 10> block_configs{{
//         {{-2.8f, 0.0f, -2.1f}, {0.6f, 0.9f, 0.6f}},
//         {{-1.6f, 0.0f, 1.8f}, {0.7f, 1.2f, 0.7f}},
//         {{-0.4f, 0.0f, -0.5f}, {0.9f, 0.7f, 0.9f}},
//         {{0.8f, 0.0f, 2.3f}, {0.5f, 1.0f, 0.5f}},
//         {{1.9f, 0.0f, -1.4f}, {0.6f, 0.8f, 0.6f}},
//         {{2.7f, 0.0f, 0.9f}, {0.5f, 1.1f, 0.5f}},
//         {{-2.0f, 0.0f, 0.2f}, {0.7f, 0.95f, 0.4f}},
//         {{0.3f, 0.0f, -2.6f}, {0.6f, 0.85f, 0.6f}},
//         {{1.2f, 0.0f, 1.2f}, {0.4f, 0.7f, 0.4f}},
//         {{-0.9f, 0.0f, 2.6f}, {0.5f, 0.8f, 0.5f}},
//     }};
//
//     auto blocks_entity_id = ecs_system.spawn(true);
//     auto& blocks_entity = ecs_system[blocks_entity_id];
//     blocks_entity.assign<ModelComponent>(mesh);
//     blocks_entity.assign<Visibility>(true);
//     blocks_entity.assign<ShadowCaster>(true);
//     blocks_entity.assign<Instances>();
//     if (auto* instances = blocks_entity.get<Instances>()) {
//         instances->clear();
//         for (auto config : block_configs) {
//             config.position[1] = kGroundTop + config.scale[1] * 0.5f;
//             instances->add(config.position, Vec3f{0.0f, 0.0f, 0.0f}, config.scale);
//         }
//     }
//
//     auto camera_entity_id = ecs_system.spawn(true);
//     auto& camera_entity = ecs_system[camera_entity_id];
//     camera_entity.assign<Transformation>();
//     if (auto* cam_transform = camera_entity.get<Transformation>()) {
//         cam_transform->set_position({0.0f, 0.0f, 5.0f});
//     }
//     camera_entity.assign<PerspectiveCamera>();
//     if (auto* camera_component = camera_entity.get<PerspectiveCamera>()) {
//         camera_component->set_viewport(window_width, window_height);
//         camera_component->set_perspective(60.0f, 0.1f, 200.0f);
//     }
//     camera_entity.assign<CameraController>();
//     camera_entity.assign<Visibility>();
//     renderer.set_active_camera(camera_entity_id);
//
//     auto point_entity_id = ecs_system.spawn(true);
//     auto& point_entity = ecs_system[point_entity_id];
//     point_entity.assign<Transformation>();
//     if (auto* transform = point_entity.get<Transformation>()) {
//         transform->set_position({0.0f, 1.5f, 0.0f});
//     }
//     point_entity.assign<PointLight>();
//     if (auto* point_light = point_entity.get<PointLight>()) {
//         point_light->color = {1.0f, 0.95f, 0.85f};
//         point_light->intensity = 60.0f;
//         point_light->radius = 25.0f;
//         point_light->shadow_near = 0.1f;
//         point_light->shadow_far = 30.0f;
//         point_light->shadow_resolution = 1024;
//         point_light->casts_shadows = true;
//     }
//     point_entity.assign<Visibility>();
//
//     auto point_entity2_id = ecs_system.spawn(true);
//     auto& point_entity2 = ecs_system[point_entity2_id];
//     point_entity2.assign<Transformation>();
//     if (auto* transform = point_entity2.get<Transformation>()) {
//         transform->set_position({-2.5f, 2.0f, 1.5f});
//     }
//     point_entity2.assign<PointLight>();
//     if (auto* point_light = point_entity2.get<PointLight>()) {
//         point_light->color = {0.4f, 0.7f, 1.0f};
//         point_light->intensity = 45.0f;
//         point_light->radius = 20.0f;
//         point_light->shadow_near = 0.1f;
//         point_light->shadow_far = 30.0f;
//         point_light->shadow_resolution = 1024;
//         point_light->casts_shadows = true;
//     }
//     point_entity2.assign<Visibility>();
//
//     auto light_marker_id = ecs_system.spawn(true);
//     auto& light_marker = ecs_system[light_marker_id];
//     light_marker.assign<ModelComponent>(mesh, false, false, true);
//     light_marker.assign<Visibility>(true);
//     light_marker.assign<ShadowCaster>(false);
//     light_marker.assign<Transparency>(true);
//     light_marker.assign<Instances>();
//     if (auto* instances = light_marker.get<Instances>()) {
//         instances->clear();
//         instances->add(Vec3f{0.0f, 1.5f, 0.0f}, Vec3f{0.0f, 0.0f, 0.0f}, Vec3f{0.25f, 0.25f, 0.25f});
//     }
//
//     auto light_marker2_id = ecs_system.spawn(true);
//     auto& light_marker2 = ecs_system[light_marker2_id];
//     light_marker2.assign<ModelComponent>(mesh, false, false, true);
//     light_marker2.assign<Visibility>(true);
//     light_marker2.assign<ShadowCaster>(false);
//     light_marker2.assign<Transparency>(true);
//     light_marker2.assign<Instances>();
//     if (auto* instances = light_marker2.get<Instances>()) {
//         instances->clear();
//         instances->add(Vec3f{-2.5f, 2.0f, 1.5f}, Vec3f{0.0f, 0.0f, 0.0f}, Vec3f{0.2f, 0.2f, 0.2f});
//     }
//
//     renderer.run();
//
//     mesh->unload(resources::ResourceState::Gpu);
//     mesh->unload(resources::ResourceState::Ram);
//
//     return 0;
// }


#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>

// ==== Include your ECS ====
#include "ecs/types.h"
#include "ecs/ids.h"
#include "ecs/hash.h"
#include "ecs/component.h"
#include "ecs/entity.h"
#include "ecs/system.h"
#include "ecs/event.h"
#include "ecs/vector_recycling.h"
#include "ecs/vector_compact.h"
#include "ecs/component_entity_list.h"
#include "ecs/entity_iterator.h"
#include "ecs/entity_subset.h"
#include "ecs/ecs_base.h"
#include "ecs/ecs.h"

// #include <ecs.h>

// =============================
//      DUMMY COMPONENTS
// =============================

// Position of an object in 2D
struct Position : public ecs::ComponentOf<Position> {
    float x = 0;
    float y = 0;

    Position() = default;
    Position(float x, float y) : x(x), y(y) {}
};

// Velocity in 2D
struct Velocity : public ecs::ComponentOf<Velocity> {
    float vx = 0;
    float vy = 0;

    Velocity() = default;
    Velocity(float vx, float vy) : vx(vx), vy(vy) {}
};

// =============================
//      PHYSICS SYSTEM
// =============================

struct PhysicsSystem : public ecs::System {

    float gravity = -9.81f;  // m/s²
    float floor_y = 0.0f;    // bounce floor
    float bounce_damping = 0.8f;

    void process(ecs::ECS* ecs, double dt) override {

        // iterate over objects with Position + Velocity
        for (auto& e : ecs->each<Position, Velocity>()) {

            auto* pos = e.get<Position>();
            auto* vel = e.get<Velocity>();

            // Gravity
            vel->vy += gravity * dt;

            // Integrate
            pos->x += vel->vx * dt;
            pos->y += vel->vy * dt;

            // Bounce off the floor
            if (pos->y < floor_y) {
                pos->y = floor_y;
                vel->vy *= -bounce_damping;
            }
        }
    }
};

// =============================
//      RENDER SYSTEM
// =============================

struct PrintSystem : public ecs::System {
    void process(ecs::ECS* ecs, double dt) override {
        (void)dt;
        return;

        std::cout << "--- FRAME -------------------\n";

        for (auto& e : ecs->each<Position, Velocity>()) {
            auto* p = e.get<Position>();
            auto* v = e.get<Velocity>();

            std::cout
                << "Entity " << e.id().id
                << "   pos=(" << p->x << ", " << p->y << ")"
                << "   vel=(" << v->vx << ", " << v->vy << ")\n";
        }
    }
};

// =============================
//          MAIN
// =============================

int main() {

    std::cout << "starting ecs example" << std::endl;
    ecs::ECS ecs;
    std::cout << "created ecs" << std::endl;

    // Register systems
    ecs.create_system<PhysicsSystem>();
    ecs.create_system<PrintSystem>();

    std::cout << "created systems" << std::endl;
    // Create a few bouncing balls
    {
        auto id = ecs.spawn(true);
        std::cout << "spawned entity" << std::endl;
        auto& e = ecs.entities[id];
        std::cout << "created entity " << id.id << " " << e << std::endl;
        e.assign<Position>(0.f, 5.f);
        std::cout << "assigning position to entity " << id.id << std::endl;
        e.assign<Velocity>(1.f, 0.f);
    }

    {
        auto id = ecs.spawn(true);
        auto& e = ecs.entities[id];
        e.assign<Position>(1.5f, 8.f);
        e.assign<Velocity>(0.f, -2.f);
    }

    {
        auto id = ecs.spawn(true);
        auto& e = ecs.entities[id];
        e.assign<Position>(-2.f, 10.f);
        e.assign<Velocity>(0.5f, 1.f);
    }
    std::cout << "created entities" << std::endl;

    // Simulation loop
    using clock = std::chrono::high_resolution_clock;
    auto last = clock::now();

    for (int frame = 0; frame < 100; ++frame) {

        auto now = clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        last = now;

        ecs.process(dt);
        std::cout << dt << std::endl;

        // Simple real-time pacing (optional)
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }

    return 0;
}
