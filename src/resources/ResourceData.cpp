#include "ResourceData.h"
#include "ResourceManager.h"
#include "../logging/logging.h" // Include the logging header

ResourceData::ResourceData(const std::string& path)
    : path(path) {}

void ResourceData::set_manager(ResourceManager* manager) {
    this->manager = manager;
}

bool ResourceData::load_cpu() {
    if (loaded_cpu) {
        logging::log(1, WARNING, "CPU resource already loaded: " + path);
        return false;
    }
    bool result = load_cpu_specific();
    if (result) {
        loaded_cpu = true;
        logging::log(1, INFO, "Successfully loaded CPU resource: " + path);
    } else {
        logging::log(1, WARNING, "Failed to load CPU resource: " + path);
    }
    return result;
}

bool ResourceData::load_gpu() {
    if (loaded_gpu) {
        logging::log(1, WARNING, "GPU resource already loaded: " + path);
        return false;
    }
    if (!loaded_cpu) {
        if (!load_cpu()) {
            logging::log(1, WARNING, "Failed to load GPU resource because CPU resource could not be loaded: " + path);
            return false;
        }
    }
    bool result = load_gpu_specific();
    if (result) {
        loaded_gpu = true;
        logging::log(1, INFO, "Successfully loaded GPU resource: " + path);
    } else {
        logging::log(1, WARNING, "Failed to load GPU resource: " + path);
    }
    return result;
}

bool ResourceData::unload_cpu() {
    if (!loaded_cpu) {
        logging::log(1, WARNING, "CPU resource not loaded, cannot unload: " + path);
        return false;
    }
    bool result = unload_cpu_specific();
    if (result) {
        loaded_cpu = false;
        logging::log(1, INFO, "Successfully unloaded CPU resource: " + path);
    } else {
        logging::log(1, WARNING, "Failed to unload CPU resource: " + path);
    }
    return result;
}

bool ResourceData::unload_gpu() {
    if (!loaded_gpu) {
        logging::log(1, WARNING, "GPU resource not loaded, cannot unload: " + path);
        return false;
    }
    bool result = unload_gpu_specific();
    if (result) {
        loaded_gpu = false;
        logging::log(1, INFO, "Successfully unloaded GPU resource: " + path);
    } else {
        logging::log(1, WARNING, "Failed to unload GPU resource: " + path);
    }
    return result;
}

bool ResourceData::operator()(State state, Type type) {
    switch (type) {
        case LOAD:
            if (state == CPU) {
                return load_cpu();
            } else if (state == GPU) {
                return load_gpu();
            }
            break;
        case UNLOAD:
            if (state == CPU) {
                return unload_cpu();
            } else if (state == GPU) {
                return unload_gpu();
            }
            break;
    }
    logging::log(1, ERROR, "Invalid operation type or state for resource: " + path);
    return false;
}

void ResourceData::load(State state) {
#ifdef F3D_PARALLEL_LOADING
    if (manager) {
        manager->queue_load_operation(shared_from_this(), state);
    } else {
#endif
        operator()(state, LOAD);
#ifdef F3D_PARALLEL_LOADING
    }
#endif
}

void ResourceData::unload(State state) {
#ifdef F3D_PARALLEL_LOADING
    if (manager) {
        manager->queue_unload_operation(shared_from_this(), state);
    } else {
#endif
        operator()(state, UNLOAD);
#ifdef F3D_PARALLEL_LOADING
    }
#endif
}
