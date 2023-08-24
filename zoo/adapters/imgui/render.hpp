#pragma once
#include "imgui.h"

#include "render/Pipeline.hpp"
#include "render/scene/Command_Buffer.hpp"

#include "render/Device_Context.hpp"
#include "render/Engine.hpp"
#include "render/Swapchain.hpp"

namespace zoo::adapters::imgui {

// @TODO: Need to refine these adapters.
void imgui_render_init(render::Engine& engine, render::Device_Context& context, render::Swapchain& main_swapchain);
void imgui_render_exit();
void imgui_render_frame_render();
void imgui_render_present();

} // namespace zoo::adapters::imgui
