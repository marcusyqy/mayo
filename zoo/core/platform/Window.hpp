#pragma once
#include <functional>
#include <string_view>

#include "Input.hpp"
#include "main/Application.hpp"

#include "render/Engine.hpp"
#include "window/Detail.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "render/Swapchain.hpp"
#include <memory>

namespace zoo {

// fwd declaration
class Window;

class Window {
public:
    Window(const render::Engine& engine, const window::Traits& traits,
        window::InputCallback callback) noexcept;

    ~Window() noexcept;

    Window(Window&& other) noexcept = delete;
    Window& operator=(Window&& other) noexcept = delete;

    Window(const Window& other) noexcept = delete;
    Window& operator=(const Window& other) noexcept = delete;

    [[nodiscard]] bool is_open() const noexcept;

    void swap_buffers() noexcept;
    void close() noexcept;

    bool valid() const noexcept { return impl_ != nullptr; }
    operator bool() const noexcept { return valid(); }

    render::Swapchain& swapchain() noexcept { return swapchain_; }
    const render::Swapchain& swapchain() const noexcept { return swapchain_; }

private:
    window::Traits traits_;
    window::InputCallback callback_;
    GLFWwindow* impl_;
    bool context_set_;

    render::Swapchain swapchain_;
};

} // namespace zoo
