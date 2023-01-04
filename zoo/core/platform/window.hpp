#pragma once
#include <functional>
#include <string_view>

#include "input.hpp"
#include "main/application.hpp"

#include "render/engine.hpp"
#include "window/detail.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "render/swapchain.hpp"
#include <memory>

namespace zoo {

// fwd declaration
class window;

class window {
public:
    using context = detail::context;
    using traits = detail::traits;
    using size = detail::size;
    using input_callback = detail::input_callback;
    using factory = detail::factory;

    window(const render::engine& engine, std::shared_ptr<context> win_ctx,
        const traits& traits, input_callback callback) noexcept;

    ~window() noexcept;

    window(window&& other) noexcept = delete;
    window& operator=(window&& other) noexcept = delete;

    window(const window& other) noexcept = delete;
    window& operator=(const window& other) noexcept = delete;

    [[nodiscard]] bool is_open() const noexcept;

    void swap_buffers() noexcept;
    void close() noexcept;

    bool valid() const noexcept { return impl_ != nullptr; }
    operator bool() const noexcept { return valid(); }

    render::swapchain& swapchain() noexcept { return swapchain_; }
    const render::swapchain& swapchain() const noexcept { return swapchain_; }

private:
    std::shared_ptr<context> context_;
    traits traits_;
    input_callback callback_;
    GLFWwindow* impl_;
    bool context_set_;

    render::swapchain swapchain_;
};

} // namespace zoo
