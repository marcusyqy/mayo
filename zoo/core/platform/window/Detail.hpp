#pragma once

#include "core/fwd.hpp"
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

struct Size {
    s32 x;
    s32 y;
};

struct Traits {
    Size size;
    bool full_screen;
    std::string_view name;
};

using InputCallback = std::function<void(Window&, input::KeyCode)>;

} // namespace window
  //

namespace windows {
void poll_events() noexcept;
}
} // namespace zoo
