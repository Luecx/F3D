//
// Created by Finn Eggers on 18.06.24.
//

#ifndef F3D_RESOURCEMANAGER_H
#define F3D_RESOURCEMANAGER_H

#include <ecs.h>

#ifdef F3D_PARALLEL_LOADING
#include <thread>
#endif

struct LoadingThread {
    std::thread thread;
    bool running = false;
};

struct ResourceData : ecs::ComponentOf<ResourceData> {
    private:
    std::string path;
    bool loaded = false;

    public:

    ResourceData(const std::string& path) : path(path) {}

    void load_cpu();
    void load_gpu();

    void unload_cpu();
    void unload_gpu();
};

class ResourceManager {
    ecs::ECS* ecs;

    ecs::EntityID register_image(const std::string& path) {
        ecs::EntityID entity = ecs->spawn(true);

        return entity;
    }

    ecs::EntityID register_mesh() {

    }

    void load() {
    }
};

#endif    // F3D_RESOURCEMANAGER_H
