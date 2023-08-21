#pragma once
#include "zoo.hpp"

#include "Device_Context.hpp"
#include "Engine.hpp"
#include "core/fwd.hpp"
#include "core/window/Detail.hpp"
#include "fwd.hpp"
#include "render/scene/CommandBuffer.hpp"
#include <cstdint>

#include "Render_Pass.hpp"
#include "core/fwd.hpp"
#include "resources/Texture.hpp"
#include "sync/Fence.hpp"
#include "sync/Semaphore.hpp"

namespace zoo::render {

//@TODO: remove Renderpass from Swapchain.
class Swapchain {
public:
    using underlying_type        = VkSwapchainKHR;
    using surface_type           = VkSurfaceKHR;
    using underlying_window_type = GLFWwindow*;

    // initialize with the device
    Swapchain(render::Engine& engine, underlying_window_type glfw_window, s32 x, s32 y) noexcept;

    ~Swapchain() noexcept;

    void resize(s32 x, s32 y) noexcept;
    void reset() noexcept;

    [[nodiscard]] VkFormat format() const noexcept { return description_.surface_format.format; }

    [[nodiscard]] VkExtent2D extent() const noexcept {
        return { static_cast<u32>(size_.x), static_cast<u32>(size_.y) };
    }

    [[nodiscard]] ViewportInfo get_viewport_info() const noexcept;

    void present() noexcept;

    // TODO: remove when "abstracted"
    struct FrameInfo {
        s32 current;
        s32 count;
    };

    FrameInfo frame_info() const noexcept;

    s32 num_images() const noexcept;
    s32 current_image() const noexcept;

    // @TODO-EVENT_SYSTEM : fix this to become some proper event system

    void on_resize(std::function<void(Swapchain&, u32, u32)> resize) noexcept;

    const resources::TextureView* get_image(s32 index) const noexcept;

    scene::PresentContext current_present_context() const noexcept;

private:
    struct WindowSize {
        s32 x;
        s32 y;
    };

private:
    bool create_swapchain_and_resources() noexcept;
    void cleanup_swapchain_and_resources() noexcept;
    void cleanup_resources() noexcept;

    void create_sync_objects() noexcept;
    void cleanup_sync_objects() noexcept;
    void force_resize(s32 width, s32 height) noexcept;
    WindowSize get_new_size() const noexcept;
    void assure(VkResult result) noexcept;

private:
    VkInstance instance_        = nullptr;
    surface_type surface_       = nullptr;
    GLFWwindow* window_         = nullptr;
    underlying_type underlying_ = nullptr;
    Device_Context& context_;

    std::vector<std::function<void(Swapchain&, u32, u32)>> resize_cbs_;

    WindowSize size_ = {};
    // std::optional<WindowSize> new_size_ = {};

    struct {
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkSurfaceCapabilitiesKHR capabilities;
    } description_ = {};

    struct SyncObjects {
        sync::Semaphore image_avail;
        sync::Semaphore render_done;
    };

    // @TODO: change these all to arrays.
    // frame specific stuff
    std::vector<VkImage> images_;
    std::vector<SyncObjects> sync_objects_;
    size_t current_sync_objects_index_ = {};

    std::vector<resources::TextureView> views_;

    u32 current_frame_ = 0;
};
} // namespace zoo::render
