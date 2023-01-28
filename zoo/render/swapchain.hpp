#pragma once

#include "core/platform/window/detail.hpp"
#include "device_context.hpp"
#include "engine.hpp"
#include "fwd.hpp"
#include "render/scene/command_buffer.hpp"
#include <cstdint>

#include "renderpass.hpp"
#include "sync/fence.hpp"
#include "sync/semaphore.hpp"

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

    void resize(width_type x, width_type y) noexcept;
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

    void present() noexcept;
    void render(std::function<void(render::scene::command_buffer& command_context,
            VkRenderPassBeginInfo renderpass_info)>
            exec) noexcept;

private:
    bool create_swapchain_and_resources() noexcept;
    void cleanup_swapchain_and_resources() noexcept;
    void cleanup_resources() noexcept;

    void create_sync_objects() noexcept;
    void cleanup_sync_objects() noexcept;
    void resize_impl() noexcept;
    void assure(VkResult result) noexcept;

private:
    VkInstance instance_ = nullptr;
    surface_type surface_ = nullptr;
    GLFWwindow* window_ = nullptr;
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

    struct sync_objects {
        sync::semaphore image_avail;
        sync::semaphore render_done;
        sync::fence in_flight_fence;
    };

    // frame specific stuff
    std::vector<VkImage> images_;
    std::vector<sync_objects> sync_objects_;
    size_t current_sync_objects_index_ = {};

    std::vector<VkImageView> views_;
    std::vector<VkFramebuffer> framebuffers_;
    std::vector<render::scene::command_buffer> command_buffers_;
    class renderpass renderpass_; // has a default renderpass in swapchain

    uint32_t current_frame_ = 0; // TODO: change this
                                 //
    bool should_resize_ = false;
};
} // namespace zoo::render
