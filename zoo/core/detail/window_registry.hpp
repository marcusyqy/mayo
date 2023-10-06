#pragma once
#include "core/fwd.hpp"
#include "core/window.hpp"

struct GLFWwindow;

namespace zoo::__hiddendetail::window_registry {

GLFWwindow* create_window(
    s32 width,
    s32 height,
    const char* name,
    Window_Traits traits = Window_Traits::RESIZABLE | Window_Traits::VISIBLE) noexcept;

void destroy_window(GLFWwindow* window) noexcept;
bool is_window_alive(GLFWwindow* window) noexcept;

} // namespace zoo::__hiddendetail::window_registry
