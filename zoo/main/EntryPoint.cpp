#include "EntryPoint.hpp"
#include "core/Log.hpp"

#include "core/platform/Window.hpp"
#include "render/Engine.hpp"
#include "render/Pipeline.hpp"
#include "render/resources/Buffer.hpp"
#include "render/scene/CommandBuffer.hpp"

#include "stdx/expected.hpp"
#include <array>
#include <fstream>
#include <string_view>

#include "render/tools/ShaderCompiler.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "render/resources/Mesh.hpp"

#include "core/Array.hpp"

namespace zoo {

namespace {

struct FrameData {

};

constexpr s32 MAX_FRAMES = 3;

struct PushConstantData {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

struct GPUCameraData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 view_proj;
};

stdx::expected<std::string, std::runtime_error> read_file(
    std::string_view filename) noexcept {
    std::ifstream file{filename.data(), std::ios::ate | std::ios::binary};
    if (!file.is_open()) {
        return stdx::unexpected{std::runtime_error("unable to open file!")};
    }

    const auto file_size = static_cast<u64>(file.tellg());
    std::string buffer;
    buffer.resize(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

using ShaderBytes = std::vector<u32>;

std::pair<ShaderBytes, ShaderBytes> read_shaders() {
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

    return std::make_pair(std::move(*vertex_spirv), std::move(*fragment_spirv));
}

void init_frame_data(FrameData& frame_data) { (void)frame_data; }

FrameData& assure_up_to_date(core::Array<FrameData, MAX_FRAMES>& frame_data,
    render::Swapchain& swapchain) {
    auto [frame_curr, frame_count] = swapchain.frame_info();

    if (frame_count != frame_data.size()) {
        frame_data.resize(frame_count);
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

    // const std::vector<render::resources::Vertex> vertices = {
    //     {{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    //     {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

    const std::vector<render::resources::Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

    // we try to use uint32_t for indices
    const std::vector<u32> indices = {0, 1, 2, 2, 3, 0};

    auto [vertex_bytes, fragment_bytes] = read_shaders();
    auto vertex_buffer =
        render::resources::Buffer::start_build<render::resources::Vertex>(
            "entry point vertex buffer")
            .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
            .count(vertices.size())
            .build(context.allocator());

    vertex_buffer.map<render::resources::Vertex>(
        [&vertices](render::resources::Vertex* data) {
            std::copy(std::begin(vertices), std::end(vertices), data);
        });

    auto index_buffer = render::resources::Buffer::start_build<uint32_t>(
        "entry point index buffer")
                            .usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
                            .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                            .count(indices.size())
                            .build(context.allocator());

    index_buffer.map<uint32_t>([&indices](uint32_t* data) {
        std::copy(std::begin(indices), std::end(indices), data);
    });

    render::resources::Mesh mesh{
        context.allocator(), "static/assets/viking_room.obj"};

    render::Shader vertex_shader{context, vertex_bytes, "main"};
    render::Shader fragment_shader{context, fragment_bytes, "main"};

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

    const auto& viewport_info = swapchain.get_viewport_info();

    PushConstantData push_constant_data{};

    auto start_time = std::chrono::high_resolution_clock::now();

    while (main_window.is_open()) {
        [[maybe_unused]] auto& frame = assure_up_to_date(frame_data, swapchain);

        // TODO: add frame data in.
        auto populate_command_ctx = [&](render::scene::CommandBuffer&
                                            command_context,
                                        VkRenderPassBeginInfo renderpass_info) {
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
