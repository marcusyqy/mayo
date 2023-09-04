#include "adapters/imgui/layer.hpp"
#include "core/utils.hpp"

#include "core/window.hpp"
#include "render/engine.hpp"
#include "utility/array.hpp"

void test() {
    using namespace zoo;
    const render::engine::Info render_engine_info{ true };

    ZOO_LOG_INFO("Starting application");

    render::Engine render_engine{ render_engine_info };
    Window main_window{ 1280, 960, "Zoo" };

    // remove namespaces
    adapters::imgui::Layer layer{ render_engine, main_window };
    layer.init();

    for (bool is_window_open = true; is_window_open; Window::poll_events()) {
        for (auto event : main_window.events_this_frame()) {
            switch (event.type) {
                case Window_Event_Type::QUIT: is_window_open = false; break;
                case Window_Event_Type::KEY:
                    if (event.key_code.key == Key::escape && event.key_code.action == Action::pressed)
                        is_window_open = false;
                    break;
            }
        }

        layer.update();
        layer.render();
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
