
#include "adapters/imgui/Layer.hpp"
#include "core/Utils.hpp"

#include "core/Window.hpp"
#include "render/Engine.hpp"

void test() {
    using namespace zoo;
    const render::engine::Info render_engine_info{ true };

    ZOO_LOG_INFO("Starting application");

    render::Engine render_engine{ render_engine_info };

    auto input_callback = [](Window& win, input::KeyCode keycode) {
        if (keycode.key_ == input::Key::escape && keycode.action_ == input::Action::pressed) {
            win.close();
        }
    };

    // TODO: I think we should just merge swapchain and window
    Window main_window{ render_engine, 1280, 960, "Zoo", input_callback };

    adapters::imgui::Layer layer{ render_engine, main_window };

    layer.init();

    while (main_window.is_open()) {
        layer.update();
        layer.render();
        Window::poll_events();
    }

    layer.exit();
}

int main(int argc, char* argv[]) { // NOLINT
    static_cast<void>(argc);
    static_cast<void>(argv);

    using namespace zoo;
    core::check_memory();
    test();
    return 0;
}
