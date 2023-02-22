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

using size_type = std::uint32_t;

struct Size {
    size_type x;
    size_type y;
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

} // namespace window
} // namespace zoo
