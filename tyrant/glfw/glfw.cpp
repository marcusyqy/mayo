#include "glfw.hpp"
#include "log.hpp"

#define GL_SILENCE_DEPRECATION
#include "backends/imgui_impl_glfw.h"
#include "basic.hpp"
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <imgui.h>


namespace {

static void glfw_error_callback(int error, const char* description) {
    SIMP_LOG_ERROR("GLFW Error {}: {}", error, description);
}

class Glfw_Context {
    Glfw_Context() {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            SIMP_LOG_ERROR("Failed to initialize glfw");
            initialized = false;
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
        initialized = true;
    }
    ~Glfw_Context() {
        if (initialized) glfwTerminate();
    }

public:
    static Glfw_Context& get() {
        static Glfw_Context ctx;
        return ctx;
    }

    bool initialized = false;
};

} // namespace

GLFWwindow* create_window(s32 width, s32 height, const char* name) {
    auto& glfw = Glfw_Context::get();
    if (!glfw.initialized) return nullptr;
    // Create window with graphics context

    GLFWwindow* window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    if (window == nullptr) {
        SIMP_LOG_ERROR("GLFW window could not be created!");
        // @TODO: return something here.
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    return window;
}

