#ifndef RESOURCEMANAGER_TPP
#define RESOURCEMANAGER_TPP

#include "resource_manager.h"

template<typename DATA>
std::shared_ptr<DATA> ResourceManager::add(const std::string& path) {
    auto data = std::make_shared<DATA>(path);
    data->set_manager(this);
    auto hash = ecs::get_type_hash<DATA>();
    resources[hash].push_back(data);
    return data;
}

#endif // RESOURCEMANAGER_TPP
