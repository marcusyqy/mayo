#pragma once
#include "imgui.h"

#include "render/Pipeline.hpp"
#include "render/scene/CommandBuffer.hpp"

#include "render/DeviceContext.hpp"
#include "render/Engine.hpp"
#include "render/Swapchain.hpp"

namespace zoo::adapters::imgui {

// @TODO: Need to refine these adapters.
void imgui_init(render::Engine& engine, render::DeviceContext& context, render::Swapchain& main_swapchain);
void imgui_exit();
void imgui_frame_render();
void imgui_present();

} // namespace zoo::adapters::imgui
