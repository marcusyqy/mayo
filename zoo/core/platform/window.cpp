#include "window.hpp"
#include "core/log.hpp"

#include "render/device_context.hpp"
#include "render/fwd.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace zoo {

namespace {

static void error_callback(
    int, [[maybe_unused]] const char* description) noexcept {
    ZOO_LOG_ERROR("Error: {}", description);
}

std::optional<size_t> get_queue_index_if_physical_device_is_chosen(
    const render::utils::physical_device& physical_device,
    VkSurfaceKHR surface) noexcept {
    if (!physical_device.has_geometry_shader() &&
        physical_device.has_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        return std::nullopt;

    size_t index{};
    for (const auto& queue_properties : physical_device.queue_properties()) {
        if (queue_properties.has_graphics() &&
            physical_device.has_present(queue_properties, surface))
            return std::make_optional(index);
        ++index;
    }
    return std::nullopt;
}

} // namespace

window::context::context() noexcept {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        ZOO_LOG_ERROR("Something went wrong when initializing glfw");
        std::abort();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // No opengl
    // if constexpr (render_type == render::api::opengl) {
    //    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //}

    // disable resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

void window::context::poll_events() noexcept { glfwPollEvents(); }

void window::context::wait_for_vsync() const noexcept { glfwSwapInterval(1); }

window::window(render::engine& engine, std::shared_ptr<context> context,
    const traits& traits, input_callback callback) noexcept :
    context_{context},
    traits_{traits}, callback_{std::move(callback)},
    impl_{glfwCreateWindow(
        traits_.size_.x_, traits_.size_.y_, traits_.name_.data(), NULL, NULL)},
    context_set_{false}, instance_(engine.vk_instance()), surface_([this]() {
        VkSurfaceKHR surface{};
        VK_EXPECT_SUCCESS(
            glfwCreateWindowSurface(instance_, impl_, nullptr, &surface));
        return surface;
    }()) {

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

void window::deallocate_render_resources() noexcept {
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
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

factory::factory(std::shared_ptr<context> context) noexcept :
    context_(std::move(context)), windows_() {}

factory::~factory() noexcept = default;

window* factory::create_window(render::engine& engine, const traits& traits,
    input_callback callback) noexcept {
    windows_.emplace_back(std::make_unique<window>(
        engine, context_, traits, std::move(callback)));
    return windows_.back().get();
}
} // namespace window_detail

} // namespace zoo
