#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "../gldata/TextureData.h"
#include "../gldata/VAOData.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using Resource           = GLData;
using MeshResource       = VAOData;
using TextureResource    = TextureData;

using MeshResourcePtr    = std::shared_ptr<MeshResource>;
using TextureResourcePtr = std::shared_ptr<TextureResource>;
using ResourcePtr        = std::shared_ptr<Resource>;

using ResourceID         = unsigned int;

class ResourceManager {
    public:
    ResourceManager();
    ~ResourceManager();

    void        queue_load(std::shared_ptr<Resource> resource);
    void        queue_unload(std::shared_ptr<Resource> resource);

    ResourcePtr create(const std::string& name);
    void        remove(ResourcePtr resource);

    void        shutdown();

    // stream output to print the resources, which are loaded and which are unloaded
    friend std::ostream& operator<<(std::ostream& os, const ResourceManager& manager);

    private:
    void                                   process_queues();

    std::vector<std::shared_ptr<Resource>> resources_;

    std::queue<std::shared_ptr<Resource>>  load_queue_;
    std::queue<std::shared_ptr<Resource>>  unload_queue_;

    std::mutex                             queue_mutex_;
    std::condition_variable                cv_;
    std::thread                            worker_;
    std::atomic<bool>                      stop_worker_;
};

#endif    // RESOURCE_MANAGER_H