#pragma once
#include <functional>
#include <string_view>

#include "Input.hpp"
#include "main/Application.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo {

struct WindowSize {
    std::uint16_t x_;
    std::uint16_t y_;
};

struct WindowTraits {
    WindowSize size_;
    bool full_screen_;
    std::string_view name_;
};

struct WindowContext : std::enable_shared_from_this<WindowContext> {
public:
    WindowContext() noexcept;
    ~WindowContext() noexcept;

    bool valid() const noexcept { return valid_; }
    void poll_events() noexcept;
    void wait_for_vsync() const noexcept;

private:
    bool valid_;
};

class Window {
public:
    using InputCallback = std::function<void(Window&, input::KeyCode)>;
    Window(std::shared_ptr<WindowContext> context, const WindowTraits& traits,
        InputCallback callback) noexcept;
    ~Window() noexcept;

    Window(Window&& other) noexcept = delete;
    Window& operator=(Window&& other) noexcept = delete;

    Window(const Window& other) noexcept = delete;
    Window& operator=(const Window& other) noexcept = delete;

    [[nodiscard("querying from glfw should not discard this")]] bool
    is_open() const noexcept;

    bool is_current_context() const noexcept;
    void current_context_here() noexcept;

    void swap_buffers() noexcept;
    void close() noexcept;

public:
    bool valid() const noexcept { return impl_ != nullptr; }
    operator bool() const noexcept { return valid(); }

private:
    std::shared_ptr<WindowContext> context_;
    WindowTraits traits_;
    InputCallback callback_;
    GLFWwindow* impl_;
    bool context_set_;
};

class WindowFactory {
public:
    using ErrorCallback = std::function<void(std::string_view description)>;
    using InputCallback = Window::InputCallback;

    WindowFactory(std::shared_ptr<WindowContext> context) noexcept;
    ~WindowFactory() noexcept;

    Window* create_window(
        const WindowTraits& traits, InputCallback callback) noexcept;

private:
    std::shared_ptr<WindowContext> context_;
    std::vector<std::unique_ptr<Window>> windows_;
};

} // namespace zoo
