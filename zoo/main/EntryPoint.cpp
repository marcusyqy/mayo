
#include "EntryPoint.hpp"

#include "Utility.hpp"

#include "core/Array.hpp"
#include "core/Log.hpp"
#include "core/platform/Window.hpp"

#include "render/Engine.hpp"
#include "render/Pipeline.hpp"
#include "render/resources/Buffer.hpp"
#include "render/resources/Mesh.hpp"
#include "render/scene/CommandBuffer.hpp"

#include "stdx/expected.hpp"

#include "render/tools/ShaderCompiler.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <fstream>
#include <string_view>

namespace zoo {

namespace {

static constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

render::resources::Texture create_depth_buffer(
    u32 x, u32 y, render::resources::Allocator& allocator) noexcept {
    return render::resources::Texture::start_build("DepthBufferSwapchain")
        .format(DEPTH_FORMAT)
        .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .extent({x, y, 1})
        .allocation_type(VMA_MEMORY_USAGE_GPU_ONLY)
        .allocation_required_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .build(allocator);
}

struct FrameData {

    FrameData(render::DeviceContext& context) noexcept
        : allocator(context.allocator()), command_context(context),
          in_flight_fence(context), render_done(context) {}

    // this is a mess.
    // it should be copyable/movable.
    FrameData(const FrameData& o) = delete;
    FrameData& operator=(const FrameData& o) = delete;
    FrameData(FrameData&& o) = delete;
    FrameData& operator=(FrameData&& o) = delete;

    // render target(s).
    render::resources::Allocator& allocator;
    render::resources::Texture depth_buffer;
    render::RenderPass renderpass;
    VkFramebuffer framebuffer;

    // commands.
    render::scene::CommandBuffer command_context;

    // syncing.
    render::sync::Fence in_flight_fence;
    render::sync::Semaphore render_done;

    // methods
    void on_resize(window::Size size) noexcept {
        auto [x, y] = size;
        depth_buffer = create_depth_buffer(x, y, allocator);

        // we also need swapchain native image view for this.
        // we should learn to abstract this.
        // recreate renderpass and framebuffer.
    }
};

VkRenderPass create_renderpass(
    render::DeviceContext& context, VkFormat format) noexcept {

    VkAttachmentDescription color_attachment{};
    color_attachment.format = format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = DEPTH_FORMAT;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depth_dependency = {};
    depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    depth_dependency.dstSubpass = 0;
    depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.srcAccessMask = 0;
    depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    // is this really needed(?)
    std::array attachments = {color_attachment, depth_attachment};
    std::array dependencies = {dependency, depth_dependency};

    VkRenderPassCreateInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_info.attachmentCount = uint32_t(attachments.size());
    renderpass_info.pAttachments = attachments.data();
    renderpass_info.subpassCount = 1;
    renderpass_info.pSubpasses = &subpass;
    renderpass_info.dependencyCount = uint32_t(dependencies.size());
    renderpass_info.pDependencies = dependencies.data();

    VkRenderPass renderpass{};
    VK_EXPECT_SUCCESS(
        vkCreateRenderPass(context, &renderpass_info, nullptr, &renderpass));

    return renderpass;
}

void release_renderpass(
    render::DeviceContext& context, VkRenderPass renderpass) {
    context.release_device_resource(renderpass);
}

void init_frame_data(FrameData& frame_data) { (void)frame_data; }
void release_frame_data(FrameData& frame_data) { (void)frame_data; }

constexpr s32 MAX_FRAMES = 3;

struct PushConstantData {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

using ShaderBytes = std::vector<u32>;

struct Shaders {
    ShaderBytes vertex;
    ShaderBytes fragment;
};

Shaders read_shaders() {
    render::tools::ShaderCompiler compiler;
    auto vertex_bytes = read_file("static/shaders/test.vert");
    ZOO_ASSERT(vertex_bytes, "vertex shader must have value!");
    auto fragment_bytes = read_file("static/shaders/test.frag");
    ZOO_ASSERT(fragment_bytes, "fragment shader must have value!");

    render::tools::ShaderWork vertex_work{
        shaderc_vertex_shader, "Test.vert", *vertex_bytes};

    render::tools::ShaderWork fragment_work{
        shaderc_fragment_shader, "Test.frag", *fragment_bytes};

    auto vertex_spirv = compiler.compile(vertex_work);
    auto fragment_spirv = compiler.compile(fragment_work);
    if (!vertex_spirv) {
        spdlog::error("Vertex has error : {}", vertex_spirv.error().what());
    }

    if (!fragment_spirv) {
        spdlog::error("Fragment has error : {}", fragment_spirv.error().what());
    }

    return {.vertex = std::move(*vertex_spirv),
        .fragment = std::move(*fragment_spirv)};
}

FrameData& assure_up_to_date(core::Array<FrameData, MAX_FRAMES>& frame_data,
    render::Swapchain& swapchain, render::DeviceContext& context) {
    auto [frame_curr, frame_count] = swapchain.frame_info();

    for (auto i = frame_count; i < frame_data.size(); ++i) {
        release_frame_data(frame_data[i]);
    }

    if (frame_count != frame_data.size()) {
        frame_data.resize(frame_count, context);
    }

    for (auto i = frame_data.size(); i < frame_count; ++i) {
        // create
        init_frame_data(frame_data[i]);
    }

    return frame_data[frame_curr];
}
} // namespace

application::ExitStatus main(application::Settings args) noexcept {
    // TODO: to make runtime arguments for different stuff.
    (void)args;

    const application::Info app_context{{0, 0, 0}, "Zoo Engine Application"};
    const render::engine::Info render_engine_info{app_context, true};

    core::Array<FrameData, MAX_FRAMES> frame_data;

    ZOO_LOG_INFO("Starting application");

    render::Engine render_engine{render_engine_info};
    auto& context = render_engine.context();
    // TODO: I think we should just merge swapchain and window
    Window main_window{render_engine,
        window::Traits{window::Size{1280, 960}, false, "zoo"},
        [](Window& win, input::KeyCode keycode) {
            if (keycode.key_ == input::Key::escape &&
                keycode.action_ == input::Action::pressed) {
                win.close();
            }
        }};

    auto [vertex_bytes, fragment_bytes] = read_shaders();
    render::Shader vertex_shader{context, vertex_bytes, "main"};
    render::Shader fragment_shader{context, fragment_bytes, "main"};

    render::resources::Mesh mesh{
        context.allocator(), "static/assets/viking_room.obj"};

    auto& swapchain = main_window.swapchain();

    auto buffer_description = render::resources::Vertex::describe();
    std::array vertex_description = {
        render::VertexInputDescription{sizeof(render::resources::Vertex),
            buffer_description, VK_VERTEX_INPUT_RATE_VERTEX}};

    render::PushConstant push_constant{};
    push_constant.size = sizeof(PushConstantData);
    push_constant.offset = 0;
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    render::Pipeline pipeline{context,
        render::ShaderStagesSpecification{
            vertex_shader, fragment_shader, vertex_description},
        swapchain.get_viewport_info(), swapchain.get_renderpass(),
        &push_constant};

    PushConstantData push_constant_data{};
    auto start_time = std::chrono::high_resolution_clock::now();

    while (main_window.is_open()) {
        [[maybe_unused]] auto& frame = assure_up_to_date(frame_data, swapchain, context);

        // TODO: add frame data in.
        auto populate_command_ctx = [&](render::scene::CommandBuffer&
                                            command_context,
                                        VkRenderPassBeginInfo renderpass_info) {
            const auto& viewport_info = swapchain.get_viewport_info();
            command_context.set_viewport(viewport_info.viewport);
            command_context.set_scissor(viewport_info.scissor);

            command_context.exec(renderpass_info, [&]() {
                glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 projection = glm::perspective(
                    glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
                projection[1][1] *= -1;

                auto current_time = std::chrono::high_resolution_clock::now();
                float time =
                    std::chrono::duration<float, std::chrono::seconds::period>(
                        current_time - start_time)
                        .count();

                glm::mat4 model = glm::rotate(glm::mat4{1.0f},
                    time * glm::radians(90.0f), glm::vec3(0, 0, 1));

                // calculate final mesh matrix
                glm::mat4 mesh_matrix = projection * view * model;
                push_constant_data.render_matrix = mesh_matrix;

                command_context.bind_pipeline(pipeline).push_constants(
                    push_constant, &push_constant_data);
                command_context.bind_mesh(mesh);
                command_context.draw_indexed(1);
            });
        };

        swapchain.render(populate_command_ctx);
        swapchain.present();

        windows::poll_events();
    }

    // TODO: we can remove this after we find out how to properly tie
    // resources to each frame.
    context.wait();

    return application::ExitStatus::ok;
}

} // namespace zoo
