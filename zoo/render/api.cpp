#include "api.hpp"
#include "render/engine.hpp"
#include "render/swapchain.hpp"
#include "utility/singleton.hpp"
#include <memory>

#include "utility/registry.hpp"
#if 0
namespace zoo::render {
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

struct Context {
    render::Engine engine;
    const Window* current_rt = nullptr;
    std::vector<std::unique_ptr<Render_Details>> registry;

    Context() noexcept {}
    ~Context() noexcept {}

    Render_Details& retrieve(const Window& window) noexcept {
        auto handle = window.id();
        ZOO_ASSERT(handle >= registry.size(), "Window was never used in Api");
        auto& details = registry[handle];
        ZOO_ASSERT(details, "Window was never used in Api");
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

Context g_context;

} // namespace

void set_target(const Window& window) noexcept {
    [[maybe_unused]] auto& render_details = g_context.assure_created(window);
    g_context.current_rt                  = &window;
}

void swap_buffers(Window& window) noexcept {
    auto& render_details = g_context.retrieve(window);
    if (render_details.drawn_this_frame) render_details.swapchain.present();
    render_details.drawn_this_frame = false;
    window.swap_buffers();
}


void set_target(const Image& image) noexcept {

}

} // namespace zoo
#endif