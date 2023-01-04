#pragma once

#include "core/platform/window/detail.hpp"
#include "device_context.hpp"
#include "engine.hpp"
#include "fwd.hpp"
#include <cstdint>

namespace zoo::render {

class swapchain {
public:
    using underlying_type = VkSwapchainKHR;
    using surface_type = VkSurfaceKHR;
    using width_type = detail::window_size_type;
    using underlying_window_type = GLFWwindow*;

    // initialize with the device
    swapchain(const render::engine& engine, underlying_window_type glfw_window,
        width_type x, width_type y) noexcept;
    ~swapchain() noexcept;

    bool resize(width_type x, width_type y) noexcept;
    void reset() noexcept;

    [[nodiscard]] VkFormat format() const noexcept {
        return description_.surface_format.format;
    }

    [[nodiscard]] VkExtent2D extent() const noexcept {
        return {size_.x, size_.y};
    }

private:
    bool create_swapchain_and_resources() noexcept;
    void cleanup_swapchain_and_resources() noexcept;

private:
    VkInstance instance_ = nullptr;
    surface_type surface_ = nullptr;
    underlying_type underlying_ = nullptr;
    std::shared_ptr<device_context> context_ = nullptr;

    struct {
        width_type x;
        width_type y;
    } size_ = {};

    struct {
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkSurfaceCapabilitiesKHR capabilities;
    } description_ = {};

    std::vector<VkImage> images_;
    std::vector<VkImageView> views_;

    // context
    // std::vector<frame> frames_;
};
} // namespace zoo::render
