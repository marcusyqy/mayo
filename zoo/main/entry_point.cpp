#include "entry_point.hpp"
#include "core/log.hpp"

#include "core/platform/window.hpp"
#include "render/engine.hpp"
#include "render/pipeline.hpp"
#include "render/scene/command_buffer.hpp"

#include "stdx/expected.hpp"
#include <array>
#include <fstream>
#include <string_view>

#include "render/tools/shader_compiler.hpp"
#include <glm/glm.hpp>

namespace zoo {

namespace {

struct vertex {
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

application::exit_status main(application::settings args) noexcept {
    (void)args;

    const application::info app_context{{0, 0, 0}, "Zoo Engine Application"};
    const render::engine::info render_engine_info{app_context, true};

    ZOO_LOG_INFO("Starting application");

    std::shared_ptr<window::context> win_context =
        std::make_shared<window::context>();

    render::engine render_engine{render_engine_info};

    // TODO: I think we should just merge swapchain and window
    window main_window{render_engine, win_context,
        window::traits{window::size{1280, 960}, false, "zoo"},
        [](window& win, input::key_code keycode) {
            if (keycode.key_ == input::key::escape &&
                keycode.action_ == input::action::pressed) {
                win.close();
            }
        }};

    const std::vector<vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    auto [vertex_bytes, fragment_bytes] = []() {
        zoo::render::tools::shader_compiler compiler;
        auto vertex_bytes = read_file("static/shaders/test.vert");
        ZOO_ASSERT(vertex_bytes, "vertex shader must have value!");
        auto fragment_bytes = read_file("static/shaders/test.frag");
        ZOO_ASSERT(fragment_bytes, "fragment shader must have value!");

        render::tools::shader_work vertex_work{
            shaderc_vertex_shader, "test.vert", *vertex_bytes};
        render::tools::shader_work fragment_work{
            shaderc_fragment_shader, "test.frag", *fragment_bytes};

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

    auto context = render_engine.context();

    // these are here for now.
    render::shader vertex_shader{context, vertex_bytes, "main"};
    render::shader fragment_shader{context, fragment_bytes, "main"};

    auto& swapchain = main_window.swapchain();

    // std::array buffer_description = {
    //     render::vertex_buffer_description{
    //         render::shader_type::vec2, offsetof(struct vertex, pos)},
    //     render::vertex_buffer_description{
    //         render::shader_type::vec3, offsetof(struct vertex, color)}};
    //
    // std::array vertex_description = {
    //     render::vertex_input_description{0, sizeof(vertex_shader),
    //         buffer_description, VK_VERTEX_INPUT_RATE_VERTEX}};

    render::pipeline pipeline{context,
        render::shader_stages_specifications{vertex_shader, fragment_shader},
        swapchain.get_viewport_info(), swapchain.get_renderpass()};

    const auto& viewport_info = swapchain.get_viewport_info();

    auto populate_command_ctx =
        [&](render::scene::command_buffer& command_context,
            VkRenderPassBeginInfo renderpass_info) {
            command_context.set_viewport(viewport_info.viewport);
            command_context.set_scissor(viewport_info.scissor);
            command_context.exec(renderpass_info, [&]() {
                command_context.bind(pipeline);
                // RENDERING TRIANGLE
                command_context.draw(3, 1, 0, 0);
            });
        };

    // swapchain.for_each(populate_command_ctx);

    while (main_window.is_open()) {
        swapchain.render(populate_command_ctx);
        swapchain.present();
        win_context->poll_events();
    }

    // TODO: we can remove this after we find out how to properly tie resources
    // to each frame.
    context->wait();

    return application::exit_status::ok;
}

} // namespace zoo
