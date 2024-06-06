#include "resource_manager.h"

#include "geometry.h"
#include "image.h"
#include "resource.h"

#include <vector>

ResourceManager::ResourceManager()
    : stop_worker_(false) {
    worker_ = std::thread(&ResourceManager::process_queues, this);
}

ResourceManager::~ResourceManager() {
    shutdown();
}

void ResourceManager::queue_load(std::shared_ptr<Resource> resource) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        load_queue_.push(resource);
    }
    cv_.notify_all();
}

void ResourceManager::queue_unload(std::shared_ptr<Resource> resource) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        unload_queue_.push(resource);
    }
    cv_.notify_all();
}

void ResourceManager::process_queues() {
    while (!stop_worker_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_.wait(lock, [this] { return !load_queue_.empty() || !unload_queue_.empty() || stop_worker_; });

        while (!load_queue_.empty()) {
            auto resource = load_queue_.front();
            load_queue_.pop();
            lock.unlock();
            resource->do_load();
            lock.lock();
        }

        while (!unload_queue_.empty()) {
            auto resource = unload_queue_.front();
            unload_queue_.pop();
            lock.unlock();
            resource->do_unload();
            lock.lock();
        }
    }
}
ResourcePtr ResourceManager::create(const std::string& name) {
    // check if .obj in name, if so, create a GeometryResource
    if (name.find(".obj") != std::string::npos) {
        auto resource = std::make_shared<GeometryResource>(this, name);
        resources_.push_back(resource);
        return resource;
    } else {
        auto resource = std::make_shared<ImageResource>(this, name);
        resources_.push_back(resource);
        return resource;
    }
}

void ResourceManager::remove(ResourcePtr resource) {
    this->queue_unload(resource);
    auto it = std::find(resources_.begin(), resources_.end(), resource);
    if (it != resources_.end()) {
        resources_.erase(it);
    }
}

// shutdown
void ResourceManager::shutdown() {
    // clear the loading queue
    while (!load_queue_.empty()) {
        load_queue_.pop();
    }
    stop_worker_ = true;
    cv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }

    // check if truly all resources are unloaded
    for (auto& resource : resources_) {
        if (resource->is_loaded()) {
            resource->do_unload();
        }
    }
}

std::ostream& operator<<(std::ostream& os, const ResourceManager& manager) {
    os << "Loaded resources:" << std::endl;
    for (const auto& resource : manager.resources_) {
        if (resource->is_loaded()) {
            os << "  " << resource->get_name() << std::endl;
        }
    }
    os << "Unloaded resources:" << std::endl;
    for (const auto& resource : manager.resources_) {
        if (!resource->is_loaded()) {
            os << "  " << resource->get_name() << std::endl;
        }
    }
    return os;
}