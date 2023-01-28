#include "window.hpp"
#include "core/log.hpp"

#include "render/device_context.hpp"
#include "render/fwd.hpp"

namespace zoo {

window::window(const render::engine& engine, std::shared_ptr<context> context,
    const traits& traits, input_callback callback) noexcept
    : context_{context}, traits_{traits}, callback_{std::move(callback)},
      impl_{glfwCreateWindow(
          traits_.size.x, traits_.size.y, traits_.name.data(), NULL, NULL)},
      context_set_{false},
      swapchain_(engine, impl_, traits_.size.x, traits_.size.y) {

    glfwSetWindowUserPointer(impl_, this);
    glfwSetKeyCallback(impl_, [](GLFWwindow* glfw_window, int key, int scancode,
                                  int action, int mods) {
        window* self =
            static_cast<window*>(glfwGetWindowUserPointer(glfw_window));
        self->callback_(*self,
            input::glfw_layer::convert(
                input::glfw_layer::key_code{key, scancode, action, mods}));
    });

    glfwSetFramebufferSizeCallback(
        impl_, []([[maybe_unused]] GLFWwindow* glfw_window, int width, int height) {
            window* self =
                static_cast<window*>(glfwGetWindowUserPointer(glfw_window));
            self->swapchain_.resize(
                static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        });
}

window::~window() noexcept { close(); }

bool window::is_open() const noexcept {
    return impl_ != nullptr && !glfwWindowShouldClose(impl_);
}

void window::close() noexcept {
    if (impl_ != nullptr) {
        glfwDestroyWindow(impl_);
        impl_ = nullptr;
    }
}

void window::swap_buffers() noexcept {
    // swap buffer for vulkan here.
    swapchain_.present();
}

} // namespace zoo
