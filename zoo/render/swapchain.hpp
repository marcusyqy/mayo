
#pragma once
#include "render/engine.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render {

class swapchain {
public:
    using underlying_type = VkSwapchainKHR;

    swapchain(render::engine& engine,

private:
    underlying_type underlying_;
};
} // namespace zoo::render
