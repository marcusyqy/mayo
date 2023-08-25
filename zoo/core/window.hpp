#pragma once
#include <functional>
#include <string_view>

#include "input.hpp"

#include "render/engine.hpp"

#include "render/swapchain.hpp"
#include <memory>
#include <stdx/span.hpp>
#include <vector>

struct GLFWwindow;

namespace zoo {

enum class Window_Event_Type { QUIT, KEY, RESIZE, UNKNOWN };

struct Window_Event {
    Window_Event_Type type = Window_Event_Type::UNKNOWN;

    // Input event data
    Key_Code key_code = {};

    // Resize event data
    s32 width  = -1;
    s32 height = -1;
};

struct Quit_Event : Window_Event {
    Quit_Event() : Window_Event{ Window_Event_Type::QUIT } {}
};

struct Input_Event : Window_Event {
    Input_Event(Key_Code kc) : Window_Event{ Window_Event_Type::KEY, kc } {}
};

struct Resize_Event : Window_Event {
    Resize_Event(s32 width, s32 height) : Window_Event{ Window_Event_Type::RESIZE, {}, width, height } {}
};

class Window {
public:
    explicit Window(
        render::Engine& engine,
        s32 width,
        s32 height,
        std::string_view name) noexcept;

    ~Window() noexcept;

    Window(Window&& other) noexcept                 = delete;
    Window(const Window& other) noexcept            = delete;
    Window& operator=(Window&& other) noexcept      = delete;
    Window& operator=(const Window& other) noexcept = delete;

    [[nodiscard]] bool is_open() const noexcept;

    void swap_buffers() noexcept;
    void close() noexcept;

    bool valid() const noexcept { return impl_ != nullptr; }
    operator bool() const noexcept { return valid(); }

    render::Swapchain& swapchain() noexcept { return swapchain_; }
    const render::Swapchain& swapchain() const noexcept { return swapchain_; }

    GLFWwindow* impl() noexcept { return impl_; }
    const GLFWwindow* impl() const noexcept { return impl_; }

    static void poll_events() noexcept;

    stdx::span<const Window_Event> events_this_frame() const noexcept;

private:
    s32 width_;
    s32 height_;
    std::string name_;

    std::vector<Window_Event> events_;

    GLFWwindow* impl_;

    // I want to remove this.
    render::Swapchain swapchain_;
    friend class render::Swapchain;
};

} // namespace zoo
