#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "mesh_group.h"
#include "../resource_state.h"

/**
 * @brief Manager for MeshGroup instances.
 *
 * Currently minimal:
 *  - Owns a list of groups.
 *  - Can create groups.
 *  - Can require/release a given state for all groups.
 */
class MeshGroupManager {
    public:
    MeshGroupManager() = default;

    /// Creates a new empty group and returns it.
    MeshGroup::Ptr create_group();

    /// Adds an existing group (if you built it elsewhere).
    void add_group(const MeshGroup::Ptr& group);

    /// Returns all groups.
    const std::vector<MeshGroup::Ptr>& groups() const { return groups_; }

    /// Require a given state for all groups (typically ResourceState::Gpu).
    void require_all(ResourceState state);

    /// Release a given state for all groups.
    void release_all(ResourceState state);

    /// Debug print.
    void dump_state(int indent = 0) const;

    private:
    std::vector<MeshGroup::Ptr> groups_;
    mutable std::mutex mtx_;
};
