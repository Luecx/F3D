#include <memory>
#include <string>
#include <vector>

#include "logging/logger.h"
#include "logging/logging.h"
#include "resource/resource_logging.h"
#include "resource/textures/texture_manager.h"
#include "resource/materials/material_manager.h"
#include "resource/materials/material.h"
#include "resource/mesh/mesh_manager.h"
#include "resource/mesh/mesh_group.h"
#include "resource/resource_state.h"
#include "gldata/ssbo_data.h"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>

// Simple helper to wire logging to stdout/stderr for our channels.
static void init_logging() {
    logging::Logger log;
    log.channel(reslog::TEXTURE).cout(logging::ALL).timestamp();
    log.channel(reslog::MATERIAL).cout(logging::ALL).timestamp();
    log.channel(reslog::MESH).cout(logging::ALL).timestamp();
    logging::set_logger(log);
}

int main() {
    init_logging();

    logging::log(reslog::MESH, logging::INFO, "Starting main setup");

    // Create minimal GLFW context for GL uploads.
    if (!glfwInit()) {
        logging::log(reslog::MESH, logging::ERROR, "Failed to init GLFW");
        return -1;
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Offscreen", nullptr, nullptr);
    if (!window) {
        logging::log(reslog::MESH, logging::ERROR, "Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        logging::log(reslog::MESH, logging::ERROR, "Failed to load GL with glad");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    TextureManager tex_mgr;
    MaterialManager mat_mgr;
    MeshManager mesh_mgr(&tex_mgr, &mat_mgr);

    // Load mesh (materials/textures resolved via OBJ/MTL)
    auto mesh = mesh_mgr.get("../assets/meshes/mesh_a.obj");
    mesh->require(ResourceState::Cpu);

    // Build GPU mesh group
    MeshGroup group;
    group.add_mesh(mesh);
    group.require(ResourceState::Cpu);

    logging::log(reslog::MESH, logging::INFO, "Setup complete");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
