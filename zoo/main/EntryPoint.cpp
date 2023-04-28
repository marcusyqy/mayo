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

namespace zoo {

namespace {

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct PushConstantData {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

stdx::expected<std::string, std::runtime_error> read_file(
    std::string_view filename) noexcept {
    std::ifstream file{filename.data(), std::ios::ate | std::ios::binary};
    if (!file.is_open()) {
        return stdx::unexpected{std::runtime_error("unable to open file!")};
    }

    const std::size_t file_size = static_cast<size_t>(file.tellg());
    std::string buffer;
    buffer.resize(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

} // namespace

application::ExitStatus main(application::Settings args) noexcept {
    (void)args;

    const application::Info app_context{{0, 0, 0}, "Zoo Engine Application"};
    const render::engine::Info render_engine_info{app_context, true};

    ZOO_LOG_INFO("Starting application");

    window::Context win_context{};

    render::Engine render_engine{render_engine_info};

    // TODO: I think we should just merge swapchain and window
    Window main_window{render_engine,
        window::Traits{window::Size{1280, 960}, false, "zoo"},
        [](Window& win, input::KeyCode keycode) {
            if (keycode.key_ == input::Key::escape &&
                keycode.action_ == input::Action::pressed) {
                win.close();
            }
        }};

    const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    auto [vertex_bytes, fragment_bytes] = []() {
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
            spdlog::error(
                "Fragment has error : {}", fragment_spirv.error().what());
        }

        return std::make_pair(
            std::move(*vertex_spirv), std::move(*fragment_spirv));
    }();

    auto& context = render_engine.context();

    // auto buffer = render::resources::Buffer::start_build(context.allocator())
    //                   .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    //                   .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
    //                   .size(sizeof(Vertex) * vertices.size())
    //                   .build();
    //
    // buffer.map<Vertex>([&vertices](Vertex* data) {
    //     std::copy(std::begin(vertices), std::end(vertices), data);
    // });
    //
    render::resources::Mesh monkey_mesh{
        context.allocator(), "static/models/monkey_flat.obj"};

    render::Shader vertex_shader{context, vertex_bytes, "main"};
    render::Shader fragment_shader{context, fragment_bytes, "main"};

    auto& swapchain = main_window.swapchain();

    // std::array buffer_description = {
    //     render::VertexBufferDescription{
    //         0, render::ShaderType::vec2, offsetof(Vertex, pos)},
    //     render::VertexBufferDescription{
    //         1, render::ShaderType::vec3, offsetof(Vertex, color)}};

    auto buffer_description = render::resources::Vertex::describe();
    std::array vertex_description = {render::VertexInputDescription{
        sizeof(Vertex), buffer_description, VK_VERTEX_INPUT_RATE_VERTEX}};

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

    size_t frame_counter{};
    auto populate_command_ctx =
        [&](render::scene::CommandBuffer& command_context,
            VkRenderPassBeginInfo renderpass_info) {
            command_context.set_viewport(viewport_info.viewport);
            command_context.set_scissor(viewport_info.scissor);
            command_context.exec(renderpass_info, [&]() {
                glm::vec3 cam_pos = {0.f, 0.f, -2.f};
                glm::mat4 view = glm::translate(glm::mat4(1.f), cam_pos);
                // camera projection
                glm::mat4 projection = glm::perspective(
                    glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
                projection[1][1] *= -1;
                // model rotation
                glm::mat4 model = glm::rotate(glm::mat4{1.0f},
                    glm::radians(frame_counter++ * 0.1f), glm::vec3(0, 1, 0));

                // calculate final mesh matrix
                glm::mat4 mesh_matrix = projection * view * model;
                push_constant_data.render_matrix = mesh_matrix;

                command_context.bind_pipeline(pipeline).push_constants(
                    push_constant, &push_constant_data);
                command_context.bind_vertex_buffers(&monkey_mesh.get());
                command_context.draw((uint32_t)monkey_mesh.count(), 1, 0, 0);
            });
        };

    while (main_window.is_open()) {
        swapchain.render(populate_command_ctx);
        swapchain.present();
        win_context.poll_events();
    }

    // TODO: we can remove this after we find out how to properly tie
    // resources to each frame.
    context.wait();

    return application::ExitStatus::ok;
}

} // namespace zoo
