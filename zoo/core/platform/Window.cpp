#include "Window.hpp"
#include "core/Log.hpp"

namespace zoo {

namespace {

static void error_callback(int, [[maybe_unused]] const char* description) {
    ZOO_LOG_ERROR("Error: {}", description);
}

} // namespace

window::Context::Context() noexcept : valid_{false} {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // No opengl  
    if constexpr(render_type == render::Api::opengl) {
         glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    // disable resizing for now 
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    valid_ = true;
}

void window::Context::poll_events() noexcept { glfwPollEvents(); }

void window::Context::wait_for_vsync() const noexcept { glfwSwapInterval(1); }

Window::Window(std::shared_ptr<window::Context> context,
    const window::Traits& traits, InputCallback callback) noexcept
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

void Window::swap_buffers() noexcept {
    if constexpr(window::Context::render_type == render::Api::opengl) {
        glfwSwapBuffers(impl_); 
    }
}

window::Context::~Context() noexcept { glfwTerminate(); }

window::Factory::Factory(std::shared_ptr<window::Context> context) noexcept
    : context_(std::move(context)), windows_() {}

window::Factory::~Factory() noexcept = default;

Window* window::Factory::create_window(
    const window::Traits& traits, InputCallback callback) noexcept {
    windows_.emplace_back(
        std::make_unique<Window>(context_, traits, std::move(callback)));
    return windows_.back().get();
}

} // namespace zoo
