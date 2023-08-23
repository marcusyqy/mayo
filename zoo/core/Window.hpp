#pragma once
#include <functional>
#include <string_view>

#include "Input.hpp"

#include "render/Engine.hpp"

#include "render/Swapchain.hpp"
#include <memory>

struct GLFWwindow;

namespace zoo {

enum class Window_Event_Type { QUIT, KEY, RESIZE };

struct Window_Event {
    Window_Event_Type type;
};

class Window {
public:
    using InputCallback = std::function<void(Window&, input::KeyCode)>;

    explicit Window(
        render::Engine& engine,
        s32 width,
        s32 height,
        std::string_view name,
        // @TODO: maybe we don't need this as well.
        InputCallback callback) noexcept;

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

private:
    s32 width_;
    s32 height_;
    std::string name_;
    InputCallback callback_;

    GLFWwindow* impl_;

    // I want to remove this.
    render::Swapchain swapchain_;
    friend class render::Swapchain;
};

} // namespace zoo
