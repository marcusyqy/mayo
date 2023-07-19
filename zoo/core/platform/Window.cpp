#include "Window.hpp"
#include "core/Log.hpp"

#include "render/DeviceContext.hpp"
#include "render/fwd.hpp"

namespace zoo {

Window::Window(render::Engine& engine, const window::Traits& traits, window::InputCallback callback) noexcept :
    traits_{ traits }, callback_{ std::move(callback) },
    impl_{ glfwCreateWindow(traits_.size.x, traits_.size.y, traits_.name.data(), NULL, NULL) }, context_set_{ false },
    swapchain_(engine, impl_, traits_.size.x, traits_.size.y) {

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

} // namespace zoo
