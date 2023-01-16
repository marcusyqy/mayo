#pragma once

#include "core/platform/window/detail.hpp"
#include "device_context.hpp"
#include "engine.hpp"
#include "fwd.hpp"
#include "render/scene/command_buffer.hpp"
#include <cstdint>

#include "renderpass.hpp"

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

    // renderpass& renderpass() noexcept { return renderpass_; }
    [[nodiscard]] const renderpass& get_renderpass() const noexcept {
        return renderpass_;
    }

    [[nodiscard]] viewport_info get_viewport_info() const noexcept;

    void for_each(
        std::function<void(render::scene::command_buffer& command_context,
            VkRenderPassBeginInfo renderpass_info)>
            exec) noexcept;

private:
    bool create_swapchain_and_resources() noexcept;
    void cleanup_swapchain_and_resources() noexcept;

    void begin_frame() noexcept;
    void end_frame() noexcept;

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

    // frame specific stuff
    std::vector<VkImage> images_;

    // TODO: merge all relevant stuff into a single struct
    // struct frame_details {
    //     VkImageView view;
    //     VkFramebuffer framebuffer;
    //     render::scene::command_buffer command_buffer;
    // };

    std::vector<VkImageView> views_;
    std::vector<VkFramebuffer> framebuffers_;
    std::vector<render::scene::command_buffer> command_buffers_;
    class renderpass renderpass_; // has a default renderpass in swapchain
};
} // namespace zoo::render
