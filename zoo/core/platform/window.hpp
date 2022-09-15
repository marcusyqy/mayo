#pragma once
#include <functional>
#include <string_view>

#include "input.hpp"
#include "main/application.hpp"
#include "render/fwd.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo {

// fwd declaration
class window;

namespace window_detail {

struct size {
    std::uint16_t x_;
    std::uint16_t y_;
};

struct traits {
    size size_;
    bool full_screen_;
    std::string_view name_;
};

struct context : std::enable_shared_from_this<context> {
public:
    context() noexcept;
    ~context() noexcept;

    auto valid() const noexcept -> bool { return valid_; }
    auto poll_events() noexcept -> void;
    auto wait_for_vsync() const noexcept -> void;

    static constexpr render::api render_type = render::api::vulkan;

private:
    bool valid_;
};

using input_callback = std::function<void(window&, input::key_code)>;

class factory {
public:
    using error_callback = std::function<void(std::string_view description)>;
    using input_callback = input_callback;

    factory(std::shared_ptr<context> context) noexcept;
    ~factory() noexcept;

    auto create_window(const traits& traits,
        input_callback callback) noexcept -> window*;

private:
    std::shared_ptr<context> context_;
    std::vector<std::unique_ptr<window>> windows_;
};
} // namespace window

class window {
public:
    using context = window_detail::context;
    using traits = window_detail::traits;
    using size = window_detail::size;
    using input_callback = window_detail::input_callback;
    using factory = window_detail::factory;

    window(std::shared_ptr<context> win_ctx,
        const traits& traits, input_callback callback) noexcept;
    ~window() noexcept;

    window(window&& other) noexcept = delete;
    auto operator=(window&& other) noexcept -> window& = delete;

    window(const window& other) noexcept = delete;
    auto operator=(const window& other) noexcept -> window& = delete;

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
    std::shared_ptr<context> context_;
    traits traits_;
    input_callback callback_;
    GLFWwindow* impl_;
    bool context_set_;
};

namespace window_detail {
} // namespace window
} // namespace zoo
