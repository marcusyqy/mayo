#include "window.hpp"
#include "core/log.hpp"

#include "render/device_context.hpp"
#include "render/fwd.hpp"

namespace zoo {

window::window(const render::engine& engine, std::shared_ptr<context> context,
    const traits& traits, input_callback callback) noexcept :
    context_{context},
    traits_{traits}, callback_{std::move(callback)},
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

    // TODO: look into allocations for vulkan

    // should query for suitable device from engine
    /*
       const auto& physical_devices = engine.physical_devices();

       for (auto begin = std::begin(physical_devices),
                 end = std::end(physical_devices);
            begin != end; ++begin) {
           if (auto index = get_queue_index_if_physical_device_is_chosen(
                   *begin, surface_)) {
               chosen_device =
                   engine.promote(begin, begin->queue_properties()[*index]);
               queue_index = *index;
               break;
           }
       }
       */
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

bool window::is_current_context() const noexcept {
    return impl_ == glfwGetCurrentContext();
}

void window::current_context_here() noexcept {
    if (context_set_)
        ZOO_LOG_WARN("cannot set context twice!");

    glfwMakeContextCurrent(impl_);
    context_set_ = true;
}

void window::swap_buffers() noexcept {
    // swap buffer for vulkan here.
}

} // namespace zoo
