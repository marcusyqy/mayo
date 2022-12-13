#include "window.hpp"
#include "core/log.hpp"

namespace zoo {

namespace {

static void error_callback(
    int, [[maybe_unused]] const char* description) noexcept {
    ZOO_LOG_ERROR("Error: {}", description);
}

} // namespace

window::context::context() noexcept : valid_{false} {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // No opengl
    // if constexpr (render_type == render::api::opengl) {
    //    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //}

    // disable resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    valid_ = true;
}

void window::context::poll_events() noexcept { glfwPollEvents(); }

void window::context::wait_for_vsync() const noexcept { glfwSwapInterval(1); }

window::window(std::shared_ptr<context> context, const traits& traits,
    input_callback callback) noexcept
    : context_{context}, traits_{traits}, callback_{std::move(callback)},
      impl_{glfwCreateWindow(traits_.size_.x_, traits_.size_.y_,
          traits_.name_.data(), NULL, NULL)},
      context_set_{false} {

    glfwSetWindowUserPointer(impl_, this);
    glfwSetKeyCallback(impl_, [](GLFWwindow* glfw_window, int key, int scancode,
                                  int action, int mods) {
        window* self =
            static_cast<window*>(glfwGetWindowUserPointer(glfw_window));
        self->callback_(*self,
            input::glfw_layer::convert(
                input::glfw_layer::key_code{key, scancode, action, mods}));
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

namespace window_detail {

context::~context() noexcept { glfwTerminate(); }

factory::factory(std::shared_ptr<context> context) noexcept
    : context_(std::move(context)), windows_() {}

factory::~factory() noexcept = default;

window* factory::create_window(
    const traits& traits, input_callback callback) noexcept {
    windows_.emplace_back(
        std::make_unique<window>(context_, traits, std::move(callback)));
    return windows_.back().get();
}
} // namespace window_detail

} // namespace zoo
