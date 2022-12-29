
#pragma once
#include "render/device_context.hpp"
#include "render/engine.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace zoo::render {

class swapchain {
public:
    using underlying_type = VkSwapchainKHR;
    using surface_type = VkSurfaceKHR;
    using width_type = std::intmax_t;

    // initialize with the device
    swapchain(render::engine& engine, surface_type surface, width_type x,
        width_type y) noexcept;

    bool resize(surface_type surface, width_type x, width_type y) noexcept;

private:
    underlying_type underlying_ = nullptr;
    std::shared_ptr<device_context> context_ = nullptr;

    struct size_type {
        width_type x;
        width_type y;
    } size_;

    // context
    // std::vector<frame> frames_;
};
} // namespace zoo::render
