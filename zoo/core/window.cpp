#include "window.hpp"

#define GLFW_INCLUDE_NONE
#include "core/log.hpp"
#include <GLFW/glfw3.h>

#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "utility/detail/initializer.hpp"

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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

void destruct() { glfwTerminate(); }

} // namespace detail

const utils::Initializer<> initializer{ detail::construct, detail::destruct };

} // namespace

Window::Window(render::Engine& engine, s32 width, s32 height, std::string_view name) noexcept :
    width_{ width }, height_{ height }, name_{ name },
    impl_{ glfwCreateWindow(width_, height_, name_.c_str(), NULL, NULL) }, swapchain_(engine, impl_, width_, height_) {

    // Keep quit here so that when we eventually need quit it will be at the front of the queue.
    events_.emplace_back(Quit_Event());

    glfwSetWindowUserPointer(impl_, this);
    glfwSetKeyCallback(impl_, [](GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto keycode = glfw_layer::convert(glfw_layer::Glfw_Key_Code{ key, scancode, action, mods });
        self->events_.emplace_back(Input_Event(keycode));
    });

    glfwSetFramebufferSizeCallback(impl_, []([[maybe_unused]] GLFWwindow* glfw_window, int width, int height) {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        self->swapchain_.resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        self->events_.emplace_back(Resize_Event(width, height));
    });
}

Window::~Window() noexcept {
    if (impl_ != nullptr) {
        glfwDestroyWindow(impl_);
        impl_ = nullptr;
    }
}

void Window::swap_buffers() noexcept {
    // swap buffer for vulkan here.
    swapchain_.present();

    // Retain the quit event.
    events_.resize(1);
}

void Window::poll_events() noexcept {
    // Clear all the windows events.
    glfwPollEvents();
}

stdx::span<const Window_Event> Window::events_this_frame() const noexcept {
    if (!glfwWindowShouldClose(impl_)) {
        // Don't put quit into the span.
        return { events_.data() + 1, events_.size() - 1 };
    }
    return events_;
}

} // namespace zoo
