#include "entry_point.hpp"
#include "core/log.hpp"

#include "core/platform/window.hpp"
#include "render/engine.hpp"

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
    auto fragment_bytes = read_file("static/shaders/frag.spv");

    while (main_window.is_open()) {
        main_window.swap_buffers();
        win_context->poll_events();
    }

    return application::exit_status::ok;
}

} // namespace zoo
