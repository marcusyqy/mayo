#include "simp.hpp"
#include "render/engine.hpp"
#include "render/swapchain.hpp"
#include "utility/singleton.hpp"
#include <memory>

#include "utility/registry.hpp"

namespace zoo {
namespace {

struct Render_Details {
    // I want this to complain
    Render_Details(const Render_Details&)            = delete;
    Render_Details(Render_Details&&)                 = delete;
    Render_Details& operator=(const Render_Details&) = delete;
    Render_Details& operator=(Render_Details&&)      = delete;

    Render_Details(render::Engine& engine, const Window& window) noexcept :
        swapchain(engine, window.impl(), window.width(), window.height()) {}
    ~Render_Details() = default;

    render::Swapchain swapchain;
    bool drawn_this_frame = false;
};

struct Context : utils::Singleton<Context> {
    render::Engine engine;
    const Window* current_rt = nullptr;
    std::vector<std::unique_ptr<Render_Details>> registry;

    Context() noexcept {}
    ~Context() noexcept {}

    Render_Details& retrieve(const Window& window) noexcept {
        auto handle = window.id();
        ZOO_ASSERT(handle >= registry.size(), "Window was never used in Simp");
        auto& details = registry[handle];
        ZOO_ASSERT(details, "Window was never used in Simp");
        return *details;
    }

    Render_Details& assure_created(const Window& window) noexcept {
        auto handle = window.id();
        if (handle >= registry.size()) {
            registry.resize(handle + 1);
        }

        auto& details = registry[handle];
        if (!details) {
            details = std::make_unique<Render_Details>(engine, window);
        }
        return *details;
    }
};

} // namespace

void Simp::set_render_target(const Window& window) noexcept {
    auto& context                         = Context::instance();
    [[maybe_unused]] auto& render_details = context.assure_created(window);
    context.current_rt                    = &window;
}

void Simp::window_resized(const Window& window, s32 x, s32 y) noexcept {
    auto& context        = Context::instance();
    auto& render_details = context.assure_created(window);
    render_details.swapchain.resize(x, y);
}

void Simp::poll_events() noexcept {
    // @TODO: change this later.
    Window::poll_events();
    // handle events here?
}

void Simp::swap_buffers(Window& window) noexcept {
    auto& context        = Context::instance();
    auto& render_details = context.retrieve(window);
    if (render_details.drawn_this_frame) render_details.swapchain.present();
    render_details.drawn_this_frame = false;
    window.swap_buffers();
}

} // namespace zoo
