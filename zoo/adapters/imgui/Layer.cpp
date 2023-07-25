#include "Layer.hpp"
#include "adapters/imgui/Defines.hpp"
#include "adapters/tools/ShaderCompiler.hpp"
#include "backends/imgui_impl_glfw.h"
#include "render/scene/UploadContext.hpp"

namespace zoo::adapters::imgui {

namespace {
render::resources::Texture
    init_font_textures(render::scene::UploadContext& upload_ctx, render::DeviceContext& device_ctx) noexcept {
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    render::resources::Buffer scratch_buffer =
        render::resources::Buffer::start_build("ScratchBuffer for ImGui:Font", width * height * 4)
            .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .allocation_type(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
            .allocation_flag(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
            .build(device_ctx.allocator());

    auto tex = render::resources::Texture::start_build("ImGui:Font")
                   .mip(1)
                   .array(1)
                   .extent({ .width = static_cast<u32>(width), .height = static_cast<u32>(height), .depth = 1 })
                   .samples(VK_SAMPLE_COUNT_1_BIT)
                   .type(VK_IMAGE_TYPE_2D)
                   .format(VK_FORMAT_R8G8B8A8_UNORM)
                   .usage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                   .tiling(VK_IMAGE_TILING_OPTIMAL)
                   .build(device_ctx.allocator());

    upload_ctx.copy(scratch_buffer, tex);
    upload_ctx.cache(std::move(scratch_buffer));
    return tex;
}

DrawData create_draw_data(
    render::scene::UploadContext& upload_ctx,
    render::DeviceContext& device_ctx,
    const render::Swapchain& sc) noexcept {
    DrawData data{};

    data.font_texture = init_font_textures(upload_ctx, device_ctx);
    data.font_sampler = render::resources::TextureSampler::start_build()
                            .address_mode_w(VK_SAMPLER_ADDRESS_MODE_REPEAT)
                            .min_filter(VK_FILTER_LINEAR)
                            .mag_filter(VK_FILTER_LINEAR)
                            .build(device_ctx);

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

    data.pipeline =
        render::Pipeline{ device_ctx,
                          render::ShaderStagesSpecification{ vertex_shader, fragment_shader, vertex_description },
                          sc.get_viewport_info(),
                          sc.get_renderpass(),
                          binding_descriptors,
                          nullptr };

    data.descriptor_pool = render::DescriptorPool{ device_ctx };

    return data;
}

FrameData create_frame_data() noexcept {}
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
    // upload stuff here.
    draw_data_ = create_draw_data(uploader, device_ctx, window_->swapchain());

    uploader.submit();
    uploader.wait(); // wait for things to finish before destructing?
}

void Layer::exit() noexcept {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Layer::update() noexcept {
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
}

void Layer::render() noexcept {}

} // namespace zoo::adapters::imgui
