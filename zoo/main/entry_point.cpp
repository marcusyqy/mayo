#include "entry_point.hpp"
#include "core/log.hpp"

#include "core/platform/window.hpp"
#include "render/engine.hpp"
#include "render/pipeline.hpp"

#include "stdx/expected.hpp"
#include <fstream>
#include <string_view>

namespace zoo {

namespace {

stdx::expected<std::vector<char>, std::runtime_error> read_file(
    std::string_view filename) noexcept {
    std::ifstream file{filename.data(), std::ios::ate | std::ios::binary};
    if (!file.is_open()) {
        return stdx::unexpected{std::runtime_error("unable to open file!")};
    }

    const std::size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);
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

    render::engine render_engine(render_engine_info);

    window main_window{render_engine, win_context,
        window::traits{window::size{1280, 960}, false, "zoo"},
        [](window& win, input::key_code keycode) {
            if (keycode.key_ == input::key::escape &&
                keycode.action_ == input::action::pressed) {
                win.close();
            }
        }};

    // for threading
    // TODO: remove this and use a more vulkan approach

    auto vertex_bytes = read_file("static/shaders/vert.spv");
    ZOO_ASSERT(vertex_bytes, "vertex shader must have value!");
    auto fragment_bytes = read_file("static/shaders/frag.spv");
    ZOO_ASSERT(fragment_bytes, "fragment shader must have value!");

    auto context = render_engine.context();

    // these are here for now.
    render::shader vertex{context, *vertex_bytes, "main"};
    render::shader fragment{context, *fragment_bytes, "main"};

    auto& swapchain = main_window.swapchain();
    render::pipeline pipeline{context,
        render::shader_stages_specifications{vertex, fragment},
        render::viewport_info{
            VkViewport{
                0.0f,                                          // x;
                0.0f,                                          // y;
                static_cast<float>(swapchain.extent().width),  // width;
                static_cast<float>(swapchain.extent().height), // height;
                0.0f,                                          // minDepth;
                1.0f                                           // maxDepth;
            },
            VkRect2D{VkOffset2D{0, 0}, swapchain.extent()}},
        swapchain.get_renderpass()};

    while (main_window.is_open()) {
        main_window.swap_buffers();
        win_context->poll_events();
    }

    return application::exit_status::ok;
}

} // namespace zoo
