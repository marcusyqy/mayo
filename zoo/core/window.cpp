#include "window.hpp"

#define GLFW_INCLUDE_NONE
#include "core/log.hpp"
#include <GLFW/glfw3.h>

#include "utility/detail/initializer.hpp"
#include "render/device_context.hpp"
#include "render/fwd.hpp"

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

Window::Window(render::Engine& engine, s32 width, s32 height, std::string_view name, InputCallback callback) noexcept :
    width_{ width }, height_{ height }, name_{ name }, callback_{ std::move(callback) },
    impl_{ glfwCreateWindow(width_, height_, name_.c_str(), NULL, NULL) }, swapchain_(engine, impl_, width_, height_) {

    glfwSetWindowUserPointer(impl_, this);
    glfwSetKeyCallback(impl_, [](GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        self->callback_(*self, input::glfw_layer::convert(input::glfw_layer::KeyCode{ key, scancode, action, mods }));
    });

    glfwSetFramebufferSizeCallback(impl_, []([[maybe_unused]] GLFWwindow* glfw_window, int width, int height) {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        self->swapchain_.resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    });
}

Window::~Window() noexcept { close(); }

bool Window::is_open() const noexcept { return impl_ != nullptr && !glfwWindowShouldClose(impl_); }

void Window::close() noexcept {
    if (impl_ != nullptr) {
        glfwDestroyWindow(impl_);
        impl_ = nullptr;
    }
}

void Window::swap_buffers() noexcept {
    // swap buffer for vulkan here.
    swapchain_.present();
}

void Window::poll_events() noexcept { glfwPollEvents(); }

} // namespace zoo
