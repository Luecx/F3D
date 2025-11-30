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
#include "resource/mesh/submesh.h"
#include "model/mesh_group.h"
#include "resource/resource_globals.h"
#include "glad.h"
#include <GLFW/glfw3.h>

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

    auto& mat_mgr = global_material_manager();
    auto& tex_mgr = global_texture_manager();
    MeshManager mesh_mgr;


    const bool ssbo_supported = GLAD_GL_VERSION_4_3;
    if (!ssbo_supported) {
        logging::log(reslog::MESH, logging::WARNING,
                     "SSBOs not supported (need GL 4.3 or ARB_shader_storage_buffer_object). Skipping GPU uploads.");
    }
    const bool use_gpu = ssbo_supported;

    // Expected texture files (place them before running):
    // assets/textures/brick_albedo.png
    // assets/textures/brick_normal.png
    // assets/textures/metal_albedo.png
    // assets/textures/metal_normal.png
    // assets/textures/wood_albedo.png
    // assets/textures/wood_normal.png

    auto tex_brick_albedo = tex_mgr.get("../assets/textures/brick_albedo.png");
    auto tex_brick_normal = tex_mgr.get("../assets/textures/brick_normal.png");
    auto tex_metal_albedo = tex_mgr.get("../assets/textures/metal_albedo.png");
    auto tex_metal_normal = tex_mgr.get("../assets/textures/metal_normal.png");
    auto tex_wood_albedo  = tex_mgr.get("../assets/textures/wood_albedo.png");
    auto tex_wood_normal  = tex_mgr.get("../assets/textures/wood_normal.png");

    logging::log(reslog::TEXTURE, logging::INFO, "Texture manager seeded with explicit handles");

    // Build three materials with different texture sets.
    auto mat_brick = std::make_shared<Material>("Brick");
    mat_brick->properties().base_color.set_texture(tex_brick_albedo);
    mat_brick->properties().normal_map = tex_brick_normal;

    auto mat_metal = std::make_shared<Material>("Metal");
    mat_metal->properties().base_color.set_texture(tex_metal_albedo);
    mat_metal->properties().normal_map = tex_metal_normal;
    mat_metal->properties().roughness.set_constant(0.2f);
    mat_metal->properties().metallic.set_constant(1.0f);

    auto mat_wood = std::make_shared<Material>("Wood");
    mat_wood->properties().base_color.set_texture(tex_wood_albedo);
    mat_wood->properties().normal_map = tex_wood_normal;
    mat_wood->properties().roughness.set_constant(0.6f);

    logging::log(reslog::MATERIAL, logging::INFO, "Material manager seeded with three materials");

    // Register materials and remember their GPU indices.
    std::uint32_t mid_brick = static_cast<std::uint32_t>(mat_mgr.add_material(mat_brick));
    std::uint32_t mid_metal = static_cast<std::uint32_t>(mat_mgr.add_material(mat_metal));
    std::uint32_t mid_wood  = static_cast<std::uint32_t>(mat_mgr.add_material(mat_wood));

    if (use_gpu) {
        // Request GPU for materials (will pull textures too) and upload SSBO.
        mat_mgr.update_gpu_buffer();
        logging::log(reslog::MATERIAL, logging::INFO, "Material GPU buffer updated");
    }

    // Load meshes from OBJ/MTL; materials/textures resolved via managers.
    auto mesh_a = mesh_mgr.get("assets/meshes/mesh_a.obj");
    auto mesh_b = mesh_mgr.get("assets/meshes/mesh_b.obj");
    auto mesh_c = mesh_mgr.get("assets/meshes/mesh_c.obj");

    mesh_mgr.request("assets/meshes/mesh_a.obj", resources::ResourceState::Ram);
    mesh_mgr.request("assets/meshes/mesh_b.obj", resources::ResourceState::Ram);
    mesh_mgr.request("assets/meshes/mesh_c.obj", resources::ResourceState::Ram);
    logging::log(reslog::MESH, logging::INFO, "Meshes requested into RAM");

    // Build mesh group and optionally GPU buffers; resolver maps material pointers to indices.
    MeshGroup group;
    group.add_mesh(mesh_a);
    group.add_mesh(mesh_b);
    group.add_mesh(mesh_c);

    auto resolver = [&](const std::shared_ptr<Material>& m) -> std::uint32_t {
        if (!m) return 0;
        if (m == mat_brick) return mid_brick;
        if (m == mat_metal) return mid_metal;
        if (m == mat_wood)  return mid_wood;
        // fallback: add dynamically
        return static_cast<std::uint32_t>(mat_mgr.add_material(m));
    };

    if (use_gpu) {
        group.request(resources::ResourceState::Gpu, resolver);
        // Process queued GPU texture jobs on the main thread.
        tex_mgr.process_gpu_jobs();
        logging::log(reslog::TEXTURE, logging::INFO, "Processed GPU texture jobs");
    }

    // Dump states.
    tex_mgr.dump_state();
    mat_mgr.dump_state();
    mesh_mgr.dump_state();

    logging::log(reslog::MESH, logging::INFO, "Setup complete. Place textures in assets/textures as listed.");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
