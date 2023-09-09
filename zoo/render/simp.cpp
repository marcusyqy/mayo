#include "simp.hpp"
#include "render/engine.hpp"
#include "utility/singleton.hpp"
#include <memory>

namespace zoo {
namespace {

struct Impl {
    render::Engine engine;

    Impl() noexcept {}
    ~Impl() noexcept {}
};



utils::Singleton<Impl> impl;

} // namespace

void Simp::set_render_target(const Window& window) noexcept { (void)window; }

void Simp::poll_events() noexcept {}

void Simp::swap_buffers(Window& window) noexcept { (void)window; }

} // namespace zoo
