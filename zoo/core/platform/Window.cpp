#include "Window.hpp"
#include "core/Log.hpp"

namespace zoo {

namespace {

static void error_callback(int, [[maybe_unused]] const char* description) {
    ZOO_LOG_ERROR("Error: {}", description);
}

} // namespace

WindowContext::WindowContext() noexcept : valid_{false} {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    valid_ = true;
}

void WindowContext::poll_events() noexcept { glfwPollEvents(); }

void WindowContext::wait_for_vsync() const noexcept { glfwSwapInterval(1); }
Window::Window(std::shared_ptr<WindowContext> context,
    const WindowTraits& traits, InputCallback callback) noexcept
    : context_{context}, traits_{traits}, callback_{std::move(callback)},
      impl_{glfwCreateWindow(traits_.size_.x_, traits_.size_.y_,
          traits_.name_.data(), NULL, NULL)},
      context_set_{false} {
    glfwSetWindowUserPointer(impl_, this);

    glfwSetKeyCallback(impl_, [](GLFWwindow* window, int key, int scancode,
                                  int action, int mods) {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->callback_(*self,
            input::glfw_layer::convert(
                input::glfw_layer::KeyCode{key, scancode, action, mods}));
    });
}

Window::~Window() noexcept { close(); }

bool Window::is_open() const noexcept {
    return impl_ != nullptr && !glfwWindowShouldClose(impl_);
}

void Window::close() noexcept {
    if (impl_ != nullptr) {
        glfwDestroyWindow(impl_);
        impl_ = nullptr;
    }
}

bool Window::is_current_context() const noexcept {
    return impl_ == glfwGetCurrentContext();
}
void Window::current_context_here() noexcept {
    if (context_set_) {
        ZOO_LOG_WARN("cannot set context twice!");
    }
    glfwMakeContextCurrent(impl_);
    context_set_ = true;
}

void Window::swap_buffers() noexcept { glfwSwapBuffers(impl_); }

WindowContext::~WindowContext() noexcept { glfwTerminate(); }

Window* WindowFactory::create_window(
    const WindowTraits& traits, InputCallback callback) noexcept {
    windows_.emplace_back(
        std::make_unique<Window>(context_, traits, std::move(callback)));
    return windows_.back().get();
}

} // namespace zoo
