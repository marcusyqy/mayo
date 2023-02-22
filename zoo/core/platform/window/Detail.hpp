#pragma once

#include "core/platform/Input.hpp"
#include "render/Engine.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo {
// fwd declaration
class Window;
namespace window {

using window_size_type = std::uint32_t;

struct Size {
    window_size_type x;
    window_size_type y;
};

struct Traits {
    Size size;
    bool full_screen;
    std::string_view name;
};

struct Context {
public:
    Context() noexcept;
    ~Context() noexcept;

    void poll_events() noexcept;
    void wait_for_vsync() const noexcept;
};

using InputCallback = std::function<void(Window&, input::KeyCode)>;

class Factory {
public:
    using error_callback = std::function<void(std::string_view description)>;
    using input_callback = InputCallback;

    Factory(const Context& context) noexcept;
    ~Factory() noexcept;

    Window* create_window(const render::Engine& engine, const Traits& traits,
        input_callback callback) noexcept;

private:
    Context& context_;
    std::vector<std::unique_ptr<Window>> windows_;
};
} // namespace window
} // namespace zoo
