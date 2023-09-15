#include "core/utils.hpp"
#include "imgui/layer.hpp"

#include "core/window.hpp"
#include "render/engine.hpp"
#include "utility/array.hpp"
#include "simp/simp.hpp"

void simp() {
    using namespace zoo;

    ZOO_LOG_INFO("Starting application");
    Window main_window{ 1280, 960, "Zoo" };
    for (bool is_window_open = true; is_window_open; Simp::poll_events()) {
        for (auto event : main_window.events_this_frame()) {
            switch (event.type) {
                case Window_Event_Type::QUIT: is_window_open = false; break;
                case Window_Event_Type::KEY:
                    if (event.key_code.key == Key::escape && event.key_code.action == Action::pressed)
                        is_window_open = false;
                    break;
                // So that it doesn't complain lol.
                case zoo::Window_Event_Type::RESIZE:
                    Simp::window_resized(main_window, event.width, event.height);
                default: break;
            }
        }
        Simp::set_render_target(main_window);
        Simp::swap_buffers(main_window);
    }
}

void demo() {
    using namespace zoo;

    ZOO_LOG_INFO("Starting application");
    render::Engine render_engine{};
    Window main_window{ 1280, 960, "Zoo" };
    imgui::Layer layer{ render_engine, main_window };

    for (bool is_window_open = true; is_window_open; Window::poll_events()) {
        for (auto event : main_window.events_this_frame()) {
            switch (event.type) {
                case Window_Event_Type::QUIT: is_window_open = false; break;
                case Window_Event_Type::KEY:
                    if (event.key_code.key == Key::escape && event.key_code.action == Action::pressed)
                        is_window_open = false;
                    break;
                // So that it doesn't complain lol.
                case zoo::Window_Event_Type::RESIZE: [[fallthrough]];
                default: break;
            }
        }

        layer.update();
        layer.render();
    }
}

// @TODO: change this to WinMain
int main(int argc, char* argv[]) { // NOLINT
    static_cast<void>(argc);
    static_cast<void>(argv);

    using namespace zoo;
    core::check_memory();
    demo();
    return 0;
}
