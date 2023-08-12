#include "Layer.hpp"
#include "adapters/imgui/Render.hpp"
#include "adapters/tools/ShaderCompiler.hpp"

#include "backends/imgui_impl_glfw.h"

#include "render/scene/UploadContext.hpp"

namespace zoo::adapters::imgui {

Layer::Layer(render::Engine& engine, Window& window) noexcept : engine_(&engine), window_(&window) {}

Layer::~Layer() noexcept { exit(); }

void Layer::init() noexcept {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Probably Okay.
    ImGui_ImplGlfw_InitForVulkan(window_->impl(), true);
}

void Layer::exit() noexcept {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Layer::update() noexcept {
    ImGui::NewFrame();
    // Here is an example of some drawing needed.
    ImGui::ShowDemoWindow();

    // Here we need to have somewhere to add some drawing.
    ImGui::Render();
}

void Layer::render() noexcept {}

} // namespace zoo::adapters::imgui
