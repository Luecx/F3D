#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#define F3D_PARALLEL_LOADING

#include "loading_thread.h"
#include <unordered_map>
#include <vector>

class ResourceManager {
    std::unordered_map<ecs::Hash, std::vector<std::shared_ptr<ResourceData>>> resources;
#ifdef F3D_PARALLEL_LOADING
    LoadingThread loading_thread;
#endif

    public:
    ResourceManager();
    ~ResourceManager();

    template<typename DATA>
    std::shared_ptr<DATA> add(const std::string& path);

#ifdef F3D_PARALLEL_LOADING
    void queue_load_operation(const std::shared_ptr<ResourceData>& data, State state);
    void queue_unload_operation(const std::shared_ptr<ResourceData>& data, State state);
#endif
};

#include "resource_manager.tpp"

#endif // RESOURCEMANAGER_H
