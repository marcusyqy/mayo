#pragma once
#include <functional>
#include <string_view>

#include "input.hpp"
#include "main/application.hpp"
#include "render/engine.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>
#include <vulkan/vulkan.h>

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

    void poll_events() noexcept;
    void wait_for_vsync() const noexcept;
};

using input_callback = std::function<void(window&, input::key_code)>;

class factory {
public:
    using error_callback = std::function<void(std::string_view description)>;
    using input_callback = input_callback;

    factory(std::shared_ptr<context> context) noexcept;
    ~factory() noexcept;

    window* create_window(render::engine& engine, const traits& traits,
        input_callback callback) noexcept;

private:
    std::shared_ptr<context> context_;
    std::vector<std::unique_ptr<window>> windows_;
};
} // namespace window_detail

class window {
public:
    using context = window_detail::context;
    using traits = window_detail::traits;
    using size = window_detail::size;
    using input_callback = window_detail::input_callback;
    using factory = window_detail::factory;

    window(render::engine& engine, std::shared_ptr<context> win_ctx,
        const traits& traits, input_callback callback) noexcept;

    ~window() noexcept;

    window(window&& other) noexcept = delete;
    window& operator=(window&& other) noexcept = delete;

    window(const window& other) noexcept = delete;
    window& operator=(const window& other) noexcept = delete;

    [[nodiscard]] bool is_open() const noexcept;

    bool is_current_context() const noexcept;
    void current_context_here() noexcept;

    void swap_buffers() noexcept;
    void close() noexcept;

    bool valid() const noexcept { return impl_ != nullptr; }
    operator bool() const noexcept { return valid(); }

private:
    void deallocate_render_resources() noexcept;

private:
    std::shared_ptr<context> context_;
    traits traits_;
    input_callback callback_;
    GLFWwindow* impl_;
    bool context_set_;

    // TODO: evaluate if this should be placed into a vulkan presenting context
    VkInstance instance_;
    VkSurfaceKHR surface_;
};

namespace window_detail {} // namespace window_detail
} // namespace zoo
