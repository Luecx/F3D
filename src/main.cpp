#include "math/mat.h"
#include "math/transformation.h"
#include "resources/mesh_data.h"
#include "resources/resource_manager.h"
#include "logging/logging.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <chrono>


int main() {

    logging::set_logger(
        logging::Logger()
            .channel(0)
                .file_output("log.txt", logging::ALL)
                .cout(logging::WARNING)
                .cerr(logging::ERROR)
                .timestamp()
            .channel(1) // resource manager
                .file_output("log_resources.txt", logging::ALL)
                .file_output("log.txt", logging::ALL)
                .cout(logging::WARNING)
                .cerr(logging::ERROR)
                .timestamp()
    );

    ResourceManager resource_manager{};
    auto mesh_data = resource_manager.add<MeshData>("resources/models/cube.obj");
    auto mesh_data2 = resource_manager.add<MeshData>("resources/models/cube.obj");

    mesh_data->load(CPU);
    mesh_data2->load(CPU);
    // load to gpu
    mesh_data->load(GPU);
    mesh_data2->load(GPU);

    return 0;
}
