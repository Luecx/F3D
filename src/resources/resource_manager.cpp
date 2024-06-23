#include "resource_manager.h"

ResourceManager::ResourceManager() {
#ifdef F3D_PARALLEL_LOADING
    logging::log(1, INFO, "Initializing ResourceManager with parallel loading.");
#endif
}

ResourceManager::~ResourceManager() {
#ifdef F3D_PARALLEL_LOADING
    logging::log(1, INFO, "Destroying ResourceManager.");
#endif
}

#ifdef F3D_PARALLEL_LOADING
void ResourceManager::queue_load_operation(const std::shared_ptr<ResourceData>& data, State state) {
    logging::log(1, DEBUG, "Queueing load operation for resource: " + data->get_path());
    loading_thread.queue_operation({data, LOAD, state});
}

void ResourceManager::queue_unload_operation(const std::shared_ptr<ResourceData>& data, State state) {
    logging::log(1, DEBUG, "Queueing unload operation for resource: " + data->get_path());
    loading_thread.queue_operation({data, UNLOAD, state});
}
#endif
