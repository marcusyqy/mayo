#include "window_registry.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <set>

namespace zoo::__hiddendetail::window_registry {

std::set<GLFWwindow*> windows;

GLFWwindow* create_window(s32 width, s32 height, const char* name, Window_Traits traits) noexcept {
    glfwWindowHint(GLFW_RESIZABLE, (traits & Window_Traits::RESIZABLE) != Window_Traits::NONE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, (traits & Window_Traits::FOCUS_ON_SHOW) != Window_Traits::NONE);
    glfwWindowHint(GLFW_VISIBLE, (traits & Window_Traits::VISIBLE) != Window_Traits::NONE);

    GLFWwindow* window = glfwCreateWindow(width, height, name, NULL, NULL);
    windows.emplace(window);
    return window;
}

void destroy_window(GLFWwindow* window) noexcept {
    ZOO_ASSERT(is_window_alive(window));
    windows.erase(window);
    glfwDestroyWindow(window);
}

bool is_window_alive(GLFWwindow* window) noexcept { return windows.contains(window); }

} // namespace zoo::__hiddendetail::window_registry
