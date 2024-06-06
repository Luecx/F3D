#include "math/mat.h"
#include "math/transformation.h"

#define GLFW_INCLUDE_NONE
#include "components/shadows/shadow_caster.h"
#include "components/shadows/lights.h"

#include "core/glerror.h"
#include "ecs/include.h"
#include "glad.h"
#include "resources/geometry.h"
#include "resources/image.h"
#include "resources/resource_manager.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

int main() {

    if (!glfwInit())
        return -1;

    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "Error: " << description << std::endl;
    });

    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "F3D", NULL, NULL);
    std::cout << "created window" << std::endl;
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    std::cout << "setting glad" << std::endl;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "glad loaded" << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;

    ecs::ECS ecs;
    auto entity = ecs.spawn();
    entity->assign<ShadowCaster>();
    entity->assign<DirectionalLight>(Vec3f(1.0f, 1.0f, 1.0f));
    entity->activate();

    // view shadow caster fbo id
    auto shadow_caster = entity->get<ShadowCaster>();
    std::cout << shadow_caster->get_fbo()->operator GLuint() << std::endl;

    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ecs.process(currentTime - previousTime);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
        previousTime = currentTime;
    }

    ecs.destroy();

    glfwTerminate();
    return 0;
}
