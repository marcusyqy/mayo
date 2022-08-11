#include <iostream>
#include "EntryPoint.hpp"
#include <spdlog/spdlog.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo {

static void errorCallback(int error, const char* description)
{
    spdlog::error("Error: {}", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

Application::ExitStatus Main(Application::Settings args) noexcept {
    spdlog::info("hello world! from {}", "zoo::Main!");

    GLFWwindow* window;

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
        return Application::ExitStatus::Err;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

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
