#pragma once

#include "core/window.hpp"

namespace zoo {

struct Simp {
    static void set_render_target(const Window& window) noexcept;
    static void poll_events() noexcept;
    static void swap_buffers(Window& window) noexcept;
};

} // namespace zoo
