#include "Detail.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "core/Window.hpp"
#include "core/Initializer.hpp"

namespace zoo {

namespace {
static void error_callback(int, [[maybe_unused]] const char* description) noexcept {
    ZOO_LOG_ERROR("Error: {}", description);
}

namespace detail {
void construct() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        ZOO_LOG_ERROR("Something went wrong when initializing glfw");
        std::abort();
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // disable resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

void destruct() { glfwTerminate(); }
} // namespace detail

const core::wrappers::Initializer<> initializer{ detail::construct, detail::destruct };

} // namespace

namespace windows {
void poll_events() noexcept { glfwPollEvents(); }
} // namespace windows

} // namespace zoo
