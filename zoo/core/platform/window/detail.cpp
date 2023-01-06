#include "detail.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "core/platform/window.hpp"

namespace zoo::detail {

namespace {
static void error_callback(
    int, [[maybe_unused]] const char* description) noexcept {
    ZOO_LOG_ERROR("Error: {}", description);
}
} // namespace

// context
context::context() noexcept {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        ZOO_LOG_ERROR("Something went wrong when initializing glfw");
        std::abort();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // disable resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

void context::poll_events() noexcept { glfwPollEvents(); }

void context::wait_for_vsync() const noexcept { glfwSwapInterval(1); }

context::~context() noexcept { glfwTerminate(); }

// factory
factory::factory(std::shared_ptr<context> context) noexcept
    : context_(std::move(context)), windows_() {}

factory::~factory() noexcept = default;

window* factory::create_window(const render::engine& engine,
    const traits& traits, input_callback callback) noexcept {
    windows_.emplace_back(std::make_unique<window>(
        engine, context_, traits, std::move(callback)));
    return windows_.back().get();
}
} // namespace zoo::detail
