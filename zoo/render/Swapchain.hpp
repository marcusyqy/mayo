#pragma once
#include "zoo.hpp"

#include "DeviceContext.hpp"
#include "Engine.hpp"
#include "core/fwd.hpp"
#include "core/platform/window/Detail.hpp"
#include "fwd.hpp"
#include "render/scene/CommandBuffer.hpp"
#include <cstdint>

#include "RenderPass.hpp"
#include "core/fwd.hpp"
#include "resources/Texture.hpp"
#include "sync/Fence.hpp"
#include "sync/Semaphore.hpp"

namespace zoo::render {

class Swapchain {
public:
    using underlying_type = VkSwapchainKHR;
    using surface_type = VkSurfaceKHR;
    using underlying_window_type = GLFWwindow*;

    // initialize with the device
    Swapchain(render::Engine& engine, underlying_window_type glfw_window, s32 x,
        s32 y) noexcept;

    ~Swapchain() noexcept;

    void resize(s32 x, s32 y) noexcept;
    void reset() noexcept;

    [[nodiscard]] VkFormat format() const noexcept {
        return description_.surface_format.format;
    }

    [[nodiscard]] VkExtent2D extent() const noexcept {
        return { static_cast<u32>(size_.x), static_cast<u32>(size_.y) };
    }

    // renderpass& renderpass() noexcept { return renderpass_; }
    [[nodiscard]] const RenderPass& get_renderpass() const noexcept {
        return renderpass_;
    }

    [[nodiscard]] ViewportInfo get_viewport_info() const noexcept;

    void for_each(
        std::function<void(render::scene::CommandBuffer& command_context,
            VkRenderPassBeginInfo renderpass_info)>
            exec) noexcept;

    void present() noexcept;

    void render(
        std::function<void(render::scene::CommandBuffer& command_context,
            VkRenderPassBeginInfo renderpass_info)>
            exec) noexcept;

    // TODO: remove when "abstracted"
    struct FrameInfo {
        s32 current;
        s32 count;
    };

    FrameInfo frame_info() const noexcept;

    s32 num_images() const noexcept;
    s32 current_image() const noexcept;

private:
    bool create_swapchain_and_resources() noexcept;
    void cleanup_swapchain_and_resources() noexcept;
    void cleanup_resources() noexcept;

    void create_sync_objects() noexcept;
    void cleanup_sync_objects() noexcept;
    void force_resize() noexcept;
    void assure(VkResult result) noexcept;

    resources::Texture create_depth_buffer();

private:
    VkInstance instance_ = nullptr;
    surface_type surface_ = nullptr;
    GLFWwindow* window_ = nullptr;
    underlying_type underlying_ = nullptr;
    DeviceContext& context_;

    struct {
        s32 x;
        s32 y;
    } size_ = {};

    struct {
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkSurfaceCapabilitiesKHR capabilities;
    } description_ = {};

    struct SyncObjects {
        sync::Semaphore image_avail;
        sync::Semaphore render_done;
        sync::Fence in_flight_fence;
    };

    // frame specific stuff
    std::vector<VkImage> images_;
    std::vector<resources::Texture> depth_buffers_;
    std::vector<SyncObjects> sync_objects_;
    size_t current_sync_objects_index_ = {};

    std::vector<VkImageView> views_;
    std::vector<VkFramebuffer> framebuffers_;
    std::vector<render::scene::CommandBuffer> command_buffers_;
    RenderPass renderpass_; // has a default renderpass in swapchain

    u32 current_frame_ = 0;
    bool should_resize_ = false;
    static constexpr VkFormat DEPTH_FORMAT_ = VK_FORMAT_D32_SFLOAT;
};
} // namespace zoo::render
