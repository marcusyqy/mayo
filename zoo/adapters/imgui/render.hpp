#pragma once
#include "imgui.h"

#include "render/pipeline.hpp"
#include "render/scene/command_buffer.hpp"

#include "core/window.hpp"
#include "render/device_context.hpp"
#include "render/engine.hpp"
#include "render/swapchain.hpp"

namespace zoo::adapters::imgui {

// @TODO: Need to refine these adapters.
void imgui_render_init(render::Engine& engine, render::Device_Context& context, const Window& main_window);
void imgui_render_exit();
void imgui_render_frame_render();
void imgui_render_present();

} // namespace zoo::adapters::imgui
