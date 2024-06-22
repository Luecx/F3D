#include "resource.h"
#include "resource_manager.h"

Resource::Resource(ResourceManager* manager, const std::string& name)
    : name_(name), loaded_(false), resource_manager(manager) {}

void Resource::load() {
    if (loaded_) {
        return;
    }
    if (resource_manager == nullptr) {
        return;
    }
    resource_manager->queue_load(shared_from_this());
}

void Resource::unload() {
    if (!loaded_) {
        return;
    }
    if (resource_manager == nullptr) {
        return;
    }
    resource_manager->queue_unload(shared_from_this());
}

// require and unrequire are used to keep track of how many objects are using the resource
void Resource::require() {
    ref_count++;
    // if not loaded, load it
    if (!loaded_) {
        load();
    }
}

void Resource::unrequire() {
    ref_count--;
    if (ref_count <= 0) {
        unload();
    }
}
