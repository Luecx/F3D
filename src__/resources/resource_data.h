#pragma once

#include "resource_types.h"

#include <ecs.h>

#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class ResourceManager;

struct ResourceData : public std::enable_shared_from_this<ResourceData> {
    using State = resources::ResourceState;
    using OpType = resources::ResourceOpType;

    struct DependencyRequest {
        std::weak_ptr<ResourceData> resource;
        State required_state{State::Drive};
    };

    struct ActiveDependency {
        State owning_state{State::Drive};
        std::shared_ptr<ResourceData> resource;
        State required_state{State::Drive};
    };

    struct DependencyStatus {
        State owning_state{State::Drive};
        std::string path;
        State required_state{State::Drive};
    };

  public:
    explicit ResourceData(std::string path);
    virtual ~ResourceData() = default;

    const std::string& get_path() const;
    State current_state() const;
    std::array<std::size_t, resources::kResourceStateCount> request_counts() const;
    std::vector<DependencyStatus> active_dependencies() const;

    void set_manager(ResourceManager* manager);
    ResourceManager* get_manager() const;

    bool supports_state(State state) const;

    // Reference counting style API used by the manager and dependencies.
    bool require(State state);
    void release(State state);

    // Legacy style API used by the loading thread interface.
    bool operator()(State state, OpType type);
    void load(State state);
    void unload(State state);

  protected:
    virtual bool load_to_ram() = 0;
    virtual void unload_from_ram() = 0;

    virtual bool load_to_gpu();
    virtual void unload_from_gpu();

    // Derived classes use this to define dependency requirements for each state transition.
    void register_dependency(State owning_state, const std::shared_ptr<ResourceData>& dependency, State required_state);

    void set_label(std::string label);
    std::string display_name() const;

  private:
    bool promote(State target_state);
    void demote(State target_state);

    bool acquire_dependencies(State owning_state);
    void release_dependencies(State owning_state);

    State highest_requested_state() const;

    std::string path_;
    ResourceManager* manager_{nullptr};
    std::string label_;

    mutable std::mutex state_mutex_;
    State state_{State::Drive};
    std::array<std::size_t, resources::kResourceStateCount> requests_{0, 0, 0};

    std::array<std::vector<DependencyRequest>, resources::kResourceStateCount> dependency_requirements_{};
    std::vector<ActiveDependency> active_dependencies_;
};
