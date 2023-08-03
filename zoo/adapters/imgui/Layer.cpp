#include "Layer.hpp"
#include "adapters/imgui/Defines.hpp"
#include "adapters/imgui/Render.hpp"
#include "adapters/tools/ShaderCompiler.hpp"

#include "backends/imgui_impl_glfw.h"

#include "render/scene/UploadContext.hpp"

namespace zoo::adapters::imgui {

namespace {
using PushConstantData = float[4];

DrawData create_draw_data(
    render::scene::UploadContext& upload_ctx,
    render::DeviceContext& device_ctx,
    const render::Swapchain& sc) noexcept {
    DrawData data{};

    std::array buffer_description{
        render::VertexBufferDescription{ 0, render::ShaderType::vec2, offsetof(ImDrawVert, pos) },
        render::VertexBufferDescription{ 1, render::ShaderType::vec2, offsetof(ImDrawVert, uv) },
        render::VertexBufferDescription{ 2, render::ShaderType::uvec4, offsetof(ImDrawVert, col) }
    };

    std::array vertex_description{
        render::VertexInputDescription{ sizeof(ImDrawVert), buffer_description, VK_VERTEX_INPUT_RATE_VERTEX }
    };

    tools::ShaderCompiler shader_compiler;

    render::Shader vertex_shader{ device_ctx, __glsl_shader_vert_spv, "main" };
    render::Shader fragment_shader{ device_ctx, __glsl_shader_frag_spv, "main" };

    render::BindingDescriptor binding_descriptors[] = {
        { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .count = 1, .stage = VK_SHADER_STAGE_FRAGMENT_BIT }
    };

    render::PushConstant push_constant_info{ .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                                             .offset     = 0,
                                             .size       = sizeof(PushConstantData) };

    render::PipelineCreateInfo pipeline_create_info{};

    data.pipeline = imgui_create_pipeline();
    render::Pipeline{ device_ctx,
                      render::ShaderStagesSpecification{ vertex_shader, fragment_shader, vertex_description },
                      sc.get_viewport_info(),
                      sc.get_renderpass(),
                      binding_descriptors,
                      &push_constant_info,
                      pipeline_create_info };

    data.descriptor_pool = render::DescriptorPool{ device_ctx, 1000 };

    return data;
}

FrameData create_frame_data() noexcept { return {}; }

} // namespace

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

    auto& device_ctx = engine_->context();

    // upload all data here.
    render::scene::UploadContext uploader{ device_ctx };

    draw_data_ = create_draw_data(uploader, device_ctx, window_->swapchain());

    uploader.submit();

    // wait for things to finish before destructing?
    uploader.wait();
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
