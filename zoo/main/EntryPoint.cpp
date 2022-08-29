#include "EntryPoint.hpp"
#include "core/Log.hpp"

#include "core/platform/Window.hpp"

namespace zoo {

constexpr application::Info app_context{{0, 0, 0}};

application::ExitStatus main(application::Settings) noexcept {
    ZOO_LOG_INFO("Starting application");

    std::shared_ptr<WindowContext> win_context =
        std::make_shared<WindowContext>();

    Window window{win_context, WindowTraits{WindowSize{640, 480}, false, "zoo"},
        [](Window& win, input::KeyCode keycode) {
            if (keycode.key_ == input::Key::escape &&
                keycode.action_ == input::Action::pressed) {
                win.close();
            }
        }};

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
