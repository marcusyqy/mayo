#include "core/utils.hpp"
#include "imgui/layer.hpp"

#include "core/window.hpp"
#include "render/engine.hpp"
#include "utility/array.hpp"

#include "core/macros.hpp"
#include "render/vulkan.hpp"

#if 0
void render_api_test() {
    using namespace zoo;

    ZOO_LOG_INFO("Starting simp_test");
    Window main_window{ 1280, 960, "Zoo" };
    for (bool is_window_open = true; is_window_open; Window::poll_events()) {
        for (const auto& event : main_window.events_this_frame()) {
            switch (event.type) {
                case Window_Event_Type::QUIT: is_window_open = false; break;
                case Window_Event_Type::KEY:
                    if (event.key_code.key == Key::escape && event.key_code.action == Action::pressed)
                        is_window_open = false;
                    break;
                // So that it doesn't complain lol.
                case zoo::Window_Event_Type::RESIZE:
                    // render::window_resized(main_window, event.width, event.height);
                    break;
                default: break;
            }
        }

        render::set_target(main_window);
        render::swap_buffers(main_window);
    }
}
#endif

#if 1
void demo() {
    using namespace zoo;

    ZOO_LOG_INFO("Starting demo");

    render::Engine render_engine{};
    Window main_window{ 1280, 960, "Zoo" };
    imgui::Layer layer{ render_engine, main_window };

    for (bool is_window_open = true; is_window_open; Window::poll_events()) {
        for (const auto& event : main_window.events_this_frame()) {
            switch (event.type) {
                case Window_Event_Type::QUIT: is_window_open = false; break;
                case Window_Event_Type::KEY:
                    if (event.key_code.key == Key::escape && event.key_code.action == Action::pressed)
                        is_window_open = false;
                    break;
                 case zoo::Window_Event_Type::RESIZE:
                    // figure out how imgui does it in examples.
                    //[[fallthrough]];
                    layer.resize(event.width, event.height);
                default: break;
            }
        }

        layer.update();
        layer.render();
    }
}
#endif


// @TODO: change this to WinMain
int main(int argc, char* argv[]) { // NOLINT
    static_cast<void>(argc);
    static_cast<void>(argv);

    using namespace zoo;
    core::check_memory();
    demo();
    #if 0
    Window window{ 1280, 960, "Zoo" };
    auto data = zoo::vk::allocate_render_context(window);
    defer { zoo::vk::free_render_context(data); };

    for (bool is_window_open = true; is_window_open; Window::poll_events()) {
        for (const auto& event : window.events_this_frame()) {
            switch (event.type) {
                case Window_Event_Type::QUIT: is_window_open = false; break;
                case Window_Event_Type::KEY:
                    if (event.key_code.key == Key::escape && event.key_code.action == Action::pressed)
                        is_window_open = false;
                    break;
                case zoo::Window_Event_Type::RESIZE:
                    // figure out how imgui does it in examples.
                    //[[fallthrough]];

                default: break;
            }
        }
    }
    #endif
    return 0;
}