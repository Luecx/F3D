#include "mesh_group_manager.h"

#include "../../logging/logging.h"
#include "../resource_logging.h"

MeshGroup::Ptr MeshGroupManager::create_group() {
    auto group = std::make_shared<MeshGroup>();
    {
        std::lock_guard<std::mutex> lock(mtx_);
        groups_.push_back(group);
    }
    return group;
}

void MeshGroupManager::add_group(const MeshGroup::Ptr& group) {
    if (!group) return;
    std::lock_guard<std::mutex> lock(mtx_);
    groups_.push_back(group);
}

void MeshGroupManager::require_all(ResourceState state) {
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& g : groups_) {
        if (!g) continue;
        g->require(state);
    }
}

void MeshGroupManager::release_all(ResourceState state) {
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& g : groups_) {
        if (!g) continue;
        g->release(state);
    }
}

void MeshGroupManager::dump_state(int indent) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string pad(indent, ' ');

    logging::log(reslog::MESH, logging::INFO, pad + "MeshGroups:");
    for (std::size_t i = 0; i < groups_.size(); ++i) {
        const auto& g = groups_[i];
        if (!g) continue;
        logging::log(
            reslog::MESH,
            logging::INFO,
            pad + "  [" + std::to_string(i) + "] cpu_users=" +
                std::to_string(g->cpu_users()) +
                " gpu_users=" + std::to_string(g->gpu_users()) +
                " draws=" + std::to_string(g->draws().size())
        );
    }
}
