#pragma once

#include <cassert>

#include "resource_state.h"

/**
 * @brief Base class for all resources providing ref-counted CPU/GPU state.
 *
 * States:
 *  - Drive: conceptual only (asset exists on disk, path known) – no counters.
 *  - Cpu:   in-RAM data – refcounted via cpu_users_.
 *  - Gpu:   GPU data – refcounted via gpu_users_.
 *
 * Subclasses implement impl_load(state) / impl_unload(state), which are called
 * only on transitions:
 *  - 0 -> 1 users for a state: impl_load(state)
 *  - 1 -> 0 users for a state: impl_unload(state)
 */
class ResourceBase {
    public:
    virtual ~ResourceBase() = default;

    /// Request the resource to be available at the given state.
    void require(ResourceState state) {
        switch (state) {
            case ResourceState::Drive:
                // Conceptual; no counters, no callbacks.
                return;

            case ResourceState::Cpu: {
                const int prev = cpu_users_;
                cpu_users_++;
                if (prev == 0) {
                    impl_load(ResourceState::Cpu);
                }
                return;
            }

            case ResourceState::Gpu: {
                const int prev = gpu_users_;
                gpu_users_++;
                if (prev == 0) {
                    impl_load(ResourceState::Gpu);
                }
                return;
            }
        }
    }

    /// Release one user's claim on the given state.
    void release(ResourceState state) {
        switch (state) {
            case ResourceState::Drive:
                // Releasing "Drive" doesn't make sense; ignore.
                return;

            case ResourceState::Cpu: {
                assert(cpu_users_ > 0 && "ResourceBase::release(Cpu) underflow");
                cpu_users_--;
                if (cpu_users_ == 0) {
                    impl_unload(ResourceState::Cpu);
                }
                return;
            }

            case ResourceState::Gpu: {
                assert(gpu_users_ > 0 && "ResourceBase::release(Gpu) underflow");
                gpu_users_--;
                if (gpu_users_ == 0) {
                    impl_unload(ResourceState::Gpu);
                }
                return;
            }
        }
    }

    int cpu_users() const { return cpu_users_; }
    int gpu_users() const { return gpu_users_; }

    protected:
    /// Called when transitioning from 0 -> 1 users for the given state.
    virtual void impl_load(ResourceState state) = 0;

    /// Called when transitioning from 1 -> 0 users for the given state.
    virtual void impl_unload(ResourceState state) = 0;

    int cpu_users_{0};
    int gpu_users_{0};
};
