#include "Detail.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "core/platform/Window.hpp"

namespace zoo::window {

namespace {
static void error_callback(
    int, [[maybe_unused]] const char* description) noexcept {
    ZOO_LOG_ERROR("Error: {}", description);
}
} // namespace

// context
Context::Context() noexcept {
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

void Context::poll_events() noexcept { glfwPollEvents(); }

void Context::wait_for_vsync() const noexcept { glfwSwapInterval(1); }

Context::~Context() noexcept { glfwTerminate(); }

// factory
Factory::Factory(Context& context) noexcept : context_(context), windows_() {}

Factory::~Factory() noexcept = default;

Window* Factory::create_window(const render::Engine& engine,
    const Traits& traits, input_callback callback) noexcept {
    windows_.emplace_back(std::make_unique<Window>(
        engine, context_, traits, std::move(callback)));
    return windows_.back().get();
}
} // namespace zoo::window
