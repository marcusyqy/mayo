#include "window.hpp"

#define GLFW_INCLUDE_NONE
#include "core/log.hpp"
#include <GLFW/glfw3.h>

#include "utility/initializer.hpp"
#include "utility/singleton.hpp"

#include "detail/window_registry.hpp"

namespace zoo {

namespace window_registry = __hiddendetail::window_registry;

// @TODO: implement these stuffs correctly.
struct Glfw_Callbacks {

    static Window& get_impl(GLFWwindow* window) noexcept {
        Window* impl = static_cast<Window*>(glfwGetWindowUserPointer(window));
        ZOO_ASSERT(impl);
        return *impl;
    }

    static void error(int, [[maybe_unused]] const char* description) noexcept {
        ZOO_LOG_ERROR("Error: {}", description);
    }

    static void key(GLFWwindow* window, int key, int scancode, int action, int mods) {
        ZOO_ASSERT(window);
        Window& self = get_impl(window);
        auto keycode = glfw_layer::convert(glfw_layer::Glfw_Key_Code{ key, scancode, action, mods });
        self.events_.emplace_back(Input_Event(keycode));
    }

    static void window_size([[maybe_unused]] GLFWwindow* window, int width, int height) {
        ZOO_ASSERT(window);
        Window& self = get_impl(window);
        self.events_.emplace_back(Resize_Event(width, height));
    }

    static void window_focus(GLFWwindow* window, int focused) {
        /*  @param[in] focused `GLFW_TRUE` if the window was given input focus, or
         *  `GLFW_FALSE` if it lost it. */
        ZOO_ASSERT(window);
    }

    static void cursor_enter(GLFWwindow* window, int entered) { ZOO_ASSERT(window); }

    static void cursor_pos(GLFWwindow* window, double xpos, double ypos) { ZOO_ASSERT(window); }
    static void mouse_button(GLFWwindow* window, int button, int action, int mods) { ZOO_ASSERT(window); }
    static void scroll(GLFWwindow* window, double xoffset, double yoffset) { ZOO_ASSERT(window); }
    static void character(GLFWwindow* window, unsigned int codepoint) { ZOO_ASSERT(window); }

    static void monitor(GLFWmonitor* monitor, int event) {
        static_cast<void>(monitor);
        if (event == GLFW_CONNECTED) {
            ZOO_LOG_TRACE("Monitor has connected.");
        } else if (event == GLFW_DISCONNECTED) {
            ZOO_LOG_TRACE("Monitor has disconnected.");
        } else {
            ZOO_LOG_TRACE("Something weird is happening with the monitor.");
        }
    }
};

namespace {

namespace detail {

// move this to window_registry maybe.
void construct() noexcept {
    glfwSetErrorCallback(&Glfw_Callbacks::error);
    if (!glfwInit()) {
        ZOO_LOG_ERROR("Something went wrong when initializing glfw");
        std::abort();
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwSetMonitorCallback(&Glfw_Callbacks::monitor);
}

void destruct() noexcept { glfwTerminate(); }

} // namespace detail

const utils::Initializer<> initializer{ detail::construct, detail::destruct };

} // namespace

// We want to eventually move away from using GLFW so we will need to implement our own windows abstraction for cross
// platform-ness?
Window::Window(s32 width, s32 height, std::string_view name) noexcept :
    width_{ width }, height_{ height }, name_{ name }, impl_{ window_registry::create_window(
                                                           width_,
                                                           height_,
                                                           name_.c_str(),
                                                           Window_Traits::RESIZABLE | Window_Traits::VISIBLE) } {
    // Keep quit here so that when we eventually need quit it will be at the front of the queue.
    events_.emplace_back(Quit_Event());

    install_callbacks();
}

void Window::install_callbacks() {
    glfwSetWindowUserPointer(impl_, this);
    glfwSetKeyCallback(impl_, &Glfw_Callbacks::key);
    glfwSetWindowSizeCallback(impl_, &Glfw_Callbacks::window_size);
    glfwSetWindowFocusCallback(impl_, &Glfw_Callbacks::window_focus);
    glfwSetCursorEnterCallback(impl_, &Glfw_Callbacks::cursor_enter);
    glfwSetCursorPosCallback(impl_, &Glfw_Callbacks::cursor_pos);
    glfwSetMouseButtonCallback(impl_, &Glfw_Callbacks::mouse_button);
    glfwSetScrollCallback(impl_, &Glfw_Callbacks::scroll);
    glfwSetCharCallback(impl_, &Glfw_Callbacks::character);
}

Window::~Window() noexcept {
    if (impl_ != nullptr) {
        window_registry::destroy_window(impl_);
        impl_ = nullptr;
    }
}

void Window::swap_buffers() noexcept {
    // Retain the quit event so we don't clear.
    events_.resize(1);
}

void Window::poll_events() noexcept {
    // Clear all the windows events.
    glfwPollEvents();
}

// Sounds like a stupid limitation that we have to use `const` in span.
stdx::span<const Window_Event> Window::events_this_frame() const noexcept {
    if (!glfwWindowShouldClose(impl_)) {
        // Don't put quit into the span.
        return { events_.data() + 1, events_.size() - 1 };
    }
    return events_;
}

std::string_view Window::get_clipboard_text() const { return glfwGetClipboardString(impl_); }
void Window::set_clipboard_text(std::string_view text) { glfwSetClipboardString(impl_, text.data()); }

} // namespace zoo
