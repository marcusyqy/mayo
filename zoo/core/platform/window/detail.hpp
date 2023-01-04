#pragma once

#include "core/platform/input.hpp"
#include "render/engine.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo {
// fwd declaration
class window;
namespace detail {

using window_size_type = std::uint32_t;

struct size {
    window_size_type x;
    window_size_type y;
};

struct traits {
    size size;
    bool full_screen;
    std::string_view name;
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

    window* create_window(const render::engine& engine, const traits& traits,
        input_callback callback) noexcept;

private:
    std::shared_ptr<context> context_;
    std::vector<std::unique_ptr<window>> windows_;
};
} // namespace detail
} // namespace zoo
