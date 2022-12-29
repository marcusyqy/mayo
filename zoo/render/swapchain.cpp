#include "swapchain.hpp"

namespace zoo::render {

swapchain::swapchain(render::engine& engine, surface_type surface, width_type x,
    width_type y) noexcept {

    // choose device first
    // call resize;
    //
    // should work correctly
    resize(surface, x, y);
}

bool swapchain::resize(
    surface_type surface, width_type x, width_type y) noexcept {
    return true;
}

} // namespace zoo::render
