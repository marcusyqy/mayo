#include "simp.hpp"
#include "render/engine.hpp"
#include "utility/singleton.hpp"
#include <memory>

namespace zoo {
namespace {

struct Context : utils::Singleton<Context> {
    render::Engine engine;

    Context() noexcept {}
    ~Context() noexcept {}
};

} // namespace

void Simp::set_render_target(const Window& window) noexcept {}

void Simp::poll_events() noexcept {
    // @TODO: change this later.
    Window::poll_events();
}

void Simp::swap_buffers(Window& window) noexcept {
    // swapbuffers for swapchain.
    window.swap_buffers();
}

} // namespace zoo
