#pragma once
#include "imgui.h"

#include "render/Pipeline.hpp"
#include "render/scene/CommandBuffer.hpp"

#include "render/DeviceContext.hpp"
#include "render/Engine.hpp"
#include "render/Swapchain.hpp"

namespace zoo::adapters::imgui {

// @Brittle this needs to be aligned with the imgui layer.
struct PushConstantData {
    float scale[2];
    float translate[2];
};

void imgui_init(render::Engine& engine, render::DeviceContext& context, render::Swapchain& main_swapchain);
void imgui_exit();

} // namespace zoo::adapters::imgui
