#pragma once
#include <functional>
#include <string_view>

#include "Input.hpp"
#include "main/Application.hpp"
#include "render/Types.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo {

namespace window {
struct Size {
    std::uint16_t x_;
    std::uint16_t y_;
};

struct Traits {
    Size size_;
    bool full_screen_;
    std::string_view name_;
};

struct Context : std::enable_shared_from_this<Context> {
public:
    Context() noexcept;
    ~Context() noexcept;

    auto valid() const noexcept -> bool { return valid_; }
    auto poll_events() noexcept -> void;
    auto wait_for_vsync() const noexcept -> void;

    static constexpr render::Api render_type = render::Api::vulkan;

private:
    bool valid_;
};
} // namespace window

class Window {
public:
    using InputCallback = std::function<void(Window&, input::KeyCode)>;
    Window(std::shared_ptr<window::Context> context,
        const window::Traits& traits, InputCallback callback) noexcept;
    ~Window() noexcept;

    Window(Window&& other) noexcept = delete;
    auto operator=(Window&& other) noexcept -> Window& = delete;

    Window(const Window& other) noexcept = delete;
    auto operator=(const Window& other) noexcept -> Window& = delete;

    [[nodiscard("querying from glfw should not discard this")]] bool
    is_open() const noexcept;

    auto is_current_context() const noexcept -> bool;
    auto current_context_here() noexcept -> void;

    auto swap_buffers() noexcept -> void;
    auto close() noexcept -> void;

public:
    auto valid() const noexcept -> bool { return impl_ != nullptr; }
    operator bool() const noexcept { return valid(); }

private:
    std::shared_ptr<window::Context> context_;
    window::Traits traits_;
    InputCallback callback_;
    GLFWwindow* impl_;
    bool context_set_;
};

namespace window {

class Factory {
public:
    using ErrorCallback = std::function<void(std::string_view description)>;
    using InputCallback = Window::InputCallback;

    Factory(std::shared_ptr<window::Context> context) noexcept;
    ~Factory() noexcept;

    auto create_window(const window::Traits& traits,
        InputCallback callback) noexcept -> Window*;

private:
    std::shared_ptr<window::Context> context_;
    std::vector<std::unique_ptr<Window>> windows_;
};
} // namespace window
} // namespace zoo
