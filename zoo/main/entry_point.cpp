#include "entry_point.hpp"
#include "core/log.hpp"

#include "core/platform/window.hpp"
#include "render/engine.hpp"

namespace zoo {

application::exit_status main(application::settings args) noexcept {
    (void)args;

    const application::info app_context{{0, 0, 0}, "Zoo Engine Application"};
    const render::engine::info render_engine_info{app_context, true};

    ZOO_LOG_INFO("Starting application");

    std::shared_ptr<window::context> win_context =
        std::make_shared<window::context>();

    render::engine render_engine(render_engine_info);

    window main_window{render_engine, win_context,
        window::traits{window::size{640, 480}, false, "zoo"},
        [](window& win, input::key_code keycode) {
            if (keycode.key_ == input::key::escape &&
                keycode.action_ == input::action::pressed) {
                win.close();
            }
        }};

    // for threading
    // TODO: remove this and use a more vulkan approach
    if (!main_window.is_current_context()) {
        main_window.current_context_here();
    }

    win_context->wait_for_vsync();

    while (main_window.is_open()) {
        main_window.swap_buffers();
        win_context->poll_events();
    }

    return application::exit_status::ok;
}

} // namespace zoo
