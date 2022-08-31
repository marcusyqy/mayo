#include "EntryPoint.hpp"
#include "core/Log.hpp"

#include "core/platform/Window.hpp"
#include "render/Engine.hpp"

namespace zoo {


application::ExitStatus main(application::Settings) noexcept {
    const application::Info app_context{{0, 0, 0}, "Zoo Engine Application"};
    const render::engine::Info render_engine_info {app_context};

    ZOO_LOG_INFO("Starting application");

    std::shared_ptr<window::Context> win_context =
        std::make_shared<window::Context>();

    Window window{win_context, window::Traits{window::Size{640, 480}, false, "zoo"},
        [](Window& win, input::KeyCode keycode) {
            if (keycode.key_ == input::Key::escape &&
                keycode.action_ == input::Action::pressed) {
                win.close();
            }
        }};

    render::Engine render_engine(render_engine_info);
    render_engine.initialize();

    // for threading
    if (!window.is_current_context()) {
        window.current_context_here();
    }

    win_context->wait_for_vsync();

    while (window.is_open()) {
        window.swap_buffers();
        win_context->poll_events();
    }

    return application::ExitStatus::ok;
}

} // namespace zoo
