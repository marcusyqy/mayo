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

namespace zoo {

namespace {

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
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

    auto buffer = render::resources::Buffer::start_build(context.allocator())
                      .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
                      .allocation_type(VMA_MEMORY_USAGE_CPU_TO_GPU)
                      .size(sizeof(Vertex) * vertices.size())
                      .build();

    buffer.map<Vertex>([&vertices](Vertex* data) {
        std::copy(std::begin(vertices), std::end(vertices), data);
    });

    render::Shader vertex_shader{context, vertex_bytes, "main"};
    render::Shader fragment_shader{context, fragment_bytes, "main"};

    auto& swapchain = main_window.swapchain();

    std::array buffer_description = {
        render::VertexBufferDescription{
            0, render::ShaderType::vec2, offsetof(Vertex, pos)},
        render::VertexBufferDescription{
            1, render::ShaderType::vec3, offsetof(Vertex, color)}};

    std::array vertex_description = {render::VertexInputDescription{
        sizeof(Vertex), buffer_description, VK_VERTEX_INPUT_RATE_VERTEX}};

    render::Pipeline pipeline{context,
        render::ShaderStagesSpecification{
            vertex_shader, fragment_shader, vertex_description},
        swapchain.get_viewport_info(), swapchain.get_renderpass()};

    const auto& viewport_info = swapchain.get_viewport_info();

    auto populate_command_ctx =
        [&](render::scene::CommandBuffer& command_context,
            VkRenderPassBeginInfo renderpass_info) {
            command_context.set_viewport(viewport_info.viewport);
            command_context.set_scissor(viewport_info.scissor);
            command_context.exec(renderpass_info, [&]() {
                command_context.bind(pipeline);
                command_context.bind_vertex_buffers(&buffer);
                command_context.draw((uint32_t)vertices.size(), 1, 0, 0);
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
