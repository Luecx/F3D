#include "resource_data.h"

#include "resource_manager.h"

#include "../logging/logging.h"

using namespace logging;
using namespace resources;

ResourceData::ResourceData(std::string path) : path_(std::move(path)) {}

const std::string& ResourceData::get_path() const { return path_; }

void ResourceData::set_label(std::string label) { label_ = std::move(label); }

std::string ResourceData::display_name() const {
    if (label_.empty()) {
        return path_;
    }
    return label_ + ": " + path_;
}

ResourceManager* ResourceData::get_manager() const { return manager_; }

resources::ResourceState ResourceData::current_state() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return state_;
}

std::array<std::size_t, resources::kResourceStateCount> ResourceData::request_counts() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return requests_;
}

std::vector<ResourceData::DependencyStatus> ResourceData::active_dependencies() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    std::vector<DependencyStatus> snapshot;
    snapshot.reserve(active_dependencies_.size());
    for (const auto& dep : active_dependencies_) {
        if (dep.resource) {
            snapshot.push_back(DependencyStatus{dep.owning_state, dep.resource->get_path(), dep.required_state});
        }
    }
    return snapshot;
}

void ResourceData::set_manager(ResourceManager* manager) { manager_ = manager; }

bool ResourceData::supports_state(State state) const { return true; }

bool ResourceData::load_to_gpu() { return true; }

void ResourceData::unload_from_gpu() {}

void ResourceData::register_dependency(State owning_state, const std::shared_ptr<ResourceData>& dependency,
                                       State required_state) {
    if (!dependency) {
        return;
    }
    dependency_requirements_[to_index(owning_state)].push_back(DependencyRequest{dependency, required_state});
}

bool ResourceData::require(State state) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto idx = to_index(state);
    requests_[idx]++;
    return promote(state);
}

void ResourceData::release(State state) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto idx = to_index(state);
    if (requests_[idx] == 0) {
        logging::log(1, WARNING, "Release called without matching require for " + display_name());
        return;
    }
    requests_[idx]--;
    auto desired = highest_requested_state();
    if (desired < state_) {
        demote(desired);
    }
}

bool ResourceData::operator()(State state, OpType type) {
    switch (type) {
    case OpType::Load:
        return require(state);
    case OpType::Unload:
        release(state);
        return true;
    }
    logging::log(1, ERROR, "Unknown operation on resource " + display_name());
    return false;
}

void ResourceData::load(State state) {
#ifdef F3D_PARALLEL_LOADING
    if (manager_) {
        manager_->queue_load_operation(shared_from_this(), state);
        return;
    }
#endif
    require(state);
}

void ResourceData::unload(State state) {
#ifdef F3D_PARALLEL_LOADING
    if (manager_) {
        manager_->queue_unload_operation(shared_from_this(), state);
        return;
    }
#endif
    release(state);
}

bool ResourceData::promote(State target_state) {
    while (state_ < target_state) {
        State next = next_state(state_);
        if (!supports_state(next)) {
            logging::log(1, ERROR,
                         "Resource does not support state " + std::string(to_string(next)) + ": " + display_name());
            return false;
        }

        if (!acquire_dependencies(next)) {
            return false;
        }

        bool success = true;
        if (next == State::Ram) {
            success = load_to_ram();
        } else if (next == State::Gpu) {
            success = load_to_gpu();
        }

        if (!success) {
            logging::log(1, ERROR, "Failed to load " + std::string(to_string(next)) + " data for " + display_name());
            release_dependencies(next);
            return false;
        }

        state_ = next;
        logging::log(1, INFO, "Resource state advanced to " + std::string(to_string(state_)) + ": " + display_name());
    }
    return true;
}

void ResourceData::demote(State target_state) {
    while (state_ > target_state) {
        State current = state_;
        if (current == State::Gpu) {
            unload_from_gpu();
        } else if (current == State::Ram) {
            unload_from_ram();
        }
        release_dependencies(current);
        state_ = previous_state(current);
        logging::log(1, INFO, "Resource state lowered to " + std::string(to_string(state_)) + ": " + display_name());
    }
}

bool ResourceData::acquire_dependencies(State owning_state) {
    auto index = to_index(owning_state);
    for (auto& requirement : dependency_requirements_[index]) {
        auto dependency = requirement.resource.lock();
        if (!dependency) {
            logging::log(1, ERROR, "Dependency expired for resource " + display_name());
            return false;
        }
        if (!dependency->require(requirement.required_state)) {
            logging::log(1, ERROR, "Failed to promote dependency for resource " + display_name());
            // Roll back already acquired dependencies for this state.
            release_dependencies(owning_state);
            return false;
        }
        active_dependencies_.push_back(ActiveDependency{owning_state, dependency, requirement.required_state});
    }
    return true;
}

void ResourceData::release_dependencies(State owning_state) {
    auto it = active_dependencies_.begin();
    while (it != active_dependencies_.end()) {
        if (it->owning_state == owning_state) {
            it->resource->release(it->required_state);
            it = active_dependencies_.erase(it);
        } else {
            ++it;
        }
    }
}

ResourceData::State ResourceData::highest_requested_state() const {
    for (int i = static_cast<int>(State::Gpu); i >= static_cast<int>(State::Drive); --i) {
        if (requests_[static_cast<std::size_t>(i)] > 0) {
            return static_cast<State>(i);
        }
    }
    return State::Drive;
}
