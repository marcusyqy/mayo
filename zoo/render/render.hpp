#pragma once
#include "core/window.hpp"

namespace zoo::Render {

void set_target(Window& window) noexcept;
void swap_buffers() noexcept;
void check_size(u32 size) noexcept;

} // namespace zoo::Render
