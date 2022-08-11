#include <iostream>
#include "EntryPoint.hpp"
#include "Log.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo {

static void errorCallback(int error, const char* description)
{
    ZOO_LOG_ERROR("Error: {}", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

Application::ExitStatus Main(Application::Settings args) noexcept {

    ZOO_LOG_INFO("Starting application");

    GLFWwindow* window;

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
        return Application::ExitStatus::Err;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return Application::ExitStatus::Err;
    }

    glfwSetKeyCallback(window, keyCallback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        // glfwGetFramebufferSize(window, &width, &height);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return Application::ExitStatus::Ok;
}

} // namespace zoo
