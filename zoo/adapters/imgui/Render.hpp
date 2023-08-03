#pragma once
#include "imgui.h"

#include "render/Pipeline.hpp"
#include "render/scene/CommandBuffer.hpp"

namespace zoo::adapters::imgui {

// @Brittle this needs to be aligned with the imgui layer.
struct PushConstantData {
    float scale[2];
    float translate[2];
};

// create pipeline functions.
render::Pipeline imgui_create_pipeline();
void imgui_push_values(render::scene::PipelineBindContext& context, const PushConstantData& data);

void imgui_init();
void imgui_exit();

} // namespace zoo::adapters::imgui
