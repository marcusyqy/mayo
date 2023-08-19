#include "render/fwd.hpp"

#include "Swapchain.hpp"

// #define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "stdx/irange.hpp"
#include <optional>

namespace zoo::render {

namespace {

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

    SwapchainSupportDetails(const utils::PhysicalDevice& physical_device, VkSurfaceKHR surface) noexcept;
};

SwapchainSupportDetails::SwapchainSupportDetails(
    const utils::PhysicalDevice& physical_device,
    VkSurfaceKHR surface) noexcept {

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    uint32_t format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    if (format_count != 0) {
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());
    }

    uint32_t present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
    }
}

bool is_device_compatible(const SwapchainSupportDetails& details) noexcept {
    return !details.formats.empty() && !details.present_modes.empty();
}

// TODO: create a priority list to choose surface format.
// for now just hardcode this.
VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) noexcept {
    // Select Surface Format (from ImGui):
    // const VkFormat requestSurfaceImageFormat[]     = { VK_FORMAT_B8G8R8A8_UNORM,
    //                                                    VK_FORMAT_R8G8B8A8_UNORM,
    //                                                    VK_FORMAT_B8G8R8_UNORM,
    //                                                    VK_FORMAT_R8G8B8_UNORM };
    for (const auto& format : available_formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return available_formats.front();
}

VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available_presentation_modes) noexcept {
    for (const auto& mode : available_presentation_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR& capabilities, s32 width, s32 height) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual_extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    actual_extent.width =
        std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

    actual_extent.height =
        std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual_extent;
}

} // namespace
  //
Swapchain::~Swapchain() noexcept {
    // we will not call resize here because swapchain will be gracefully removed
    // using the create_info struct.
    cleanup_swapchain_and_resources();
    reset();
}

Swapchain::Swapchain(render::Engine& engine, underlying_window_type glfw_window, s32 x, s32 y) noexcept :
    instance_(engine.vk_instance()), window_(glfw_window), context_(engine.context()), sync_objects_{} {

    // create surface first
    VK_EXPECT_SUCCESS(glfwCreateWindowSurface(instance_, window_, nullptr, &surface_));

    ZOO_ASSERT(
        [this]() noexcept {
            for (const auto& queue_properties : context_.physical().queue_properties()) {
                VkBool32 is_present_supported{ VK_FALSE };
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    context_.physical(),
                    queue_properties.index(),
                    surface_,
                    &is_present_supported);

                if (is_present_supported == VK_TRUE) return true;
            }
            return false;
        }(),
        "Must support present on at least one queue!");

    size_.x = x;
    size_.y = y;
    create_swapchain_and_resources();
}

bool Swapchain::create_swapchain_and_resources() noexcept {

    // wait for device to be idle
    context_.wait();

    SwapchainSupportDetails details{ context_.physical(), surface_ };
    ZOO_ASSERT(is_device_compatible(details), "Device chosen must be compatible with the swapchain!");

    description_.surface_format = choose_surface_format(details.formats);
    description_.present_mode   = choose_present_mode(details.present_modes);
    description_.capabilities   = std::move(details.capabilities);

    VkExtent2D extent = choose_extent(description_.capabilities, size_.x, size_.y);

    // set to the right extent
    size_.x = static_cast<s32>(extent.width);
    size_.y = static_cast<s32>(extent.height);

    uint32_t image_count = std::clamp(
        description_.capabilities.minImageCount + 1,
        description_.capabilities.minImageCount,
        description_.capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR create_info{ .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                          .pNext                 = nullptr,
                                          .flags                 = 0,
                                          .surface               = surface_,
                                          .minImageCount         = image_count,
                                          .imageFormat           = description_.surface_format.format,
                                          .imageColorSpace       = description_.surface_format.colorSpace,
                                          .imageExtent           = extent,
                                          .imageArrayLayers      = 1,
                                          .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                          .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
                                          .queueFamilyIndexCount = 0,
                                          .pQueueFamilyIndices   = nullptr,
                                          .preTransform          = description_.capabilities.currentTransform,
                                          .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                          .presentMode           = description_.present_mode,
                                          .clipped               = VK_TRUE,
                                          .oldSwapchain          = underlying_ };

    bool failed = false;
    VK_EXPECT_SUCCESS(
        vkCreateSwapchainKHR(context_, &create_info, nullptr, &underlying_),
        [&failed](VkResult /* result */) { failed = true; });

    // required to safely destroy after creating new swapchain.
    if (create_info.oldSwapchain != nullptr) {
        vkDestroySwapchainKHR(context_, create_info.oldSwapchain, nullptr);
    }

    // retrieve images
    vkGetSwapchainImagesKHR(context_, underlying_, &image_count, nullptr);
    images_.resize(image_count);
    vkGetSwapchainImagesKHR(context_, underlying_, &image_count, images_.data());

    views_.clear();
    views_.reserve(image_count);

    for (const auto& image : images_) {
        VkImageViewCreateInfo view_create_info{};
        view_create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image    = image;
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format   = description_.surface_format.format;

        view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        view_create_info.subresourceRange.baseMipLevel   = 0;
        view_create_info.subresourceRange.levelCount     = 1;
        view_create_info.subresourceRange.baseArrayLayer = 0;
        view_create_info.subresourceRange.layerCount     = 1;

        views_.emplace_back("Swapchain image view", context_.logical(), view_create_info);
        // VK_EXPECT_SUCCESS(
        //     vkCreateImageView(context_, &view_create_info, nullptr, view++),
        //     [&view, this, &failed](VkResult /* result */) {
        //         ZOO_LOG_ERROR("Failed to create image views! For index", std::distance(views_.data(), view - 1));
        //         failed = true;
        //     });
    }

    // reset
    sync_objects_.clear();
    sync_objects_.reserve(std::size(images_));

    // TODO : change sizes to s32.
    for (s32 i = 0, size = static_cast<s32>(std::size(images_)); i < size; ++i) {
        sync_objects_.push_back(SyncObjects{ context_, context_ });
    }

    // reset sync object index so that we don't have to care about size of the
    // vector
    current_sync_objects_index_ = 0;
    assure(vkAcquireNextImageKHR(
        context_,
        underlying_,
        std::numeric_limits<std::uint64_t>::max(),
        sync_objects_[current_sync_objects_index_].image_avail,
        nullptr,
        &current_frame_));

    return !failed;
}

void Swapchain::cleanup_resources() noexcept {
    // wait for resources to be done
    context_.wait();
    views_.clear();

    sync_objects_.clear();
    images_.clear();
}

void Swapchain::cleanup_swapchain_and_resources() noexcept {
    // not waiting here because cleanup waits
    cleanup_resources();
    vkDestroySwapchainKHR(context_, underlying_, nullptr);
    underlying_ = nullptr;
}

void Swapchain::resize(s32 x, s32 y) noexcept { force_resize(x, y); }

Swapchain::WindowSize Swapchain::get_new_size() const noexcept {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window_, &width, &height);
        // TODO: This may actually cause problems when having multiple windows
        glfwWaitEvents(); // this should be done outside (maybe)
    }
    glfwGetFramebufferSize(window_, &width, &height);
    return { static_cast<s32>(width), static_cast<s32>(height) };
}

void Swapchain::force_resize(s32 width, s32 height) noexcept {
    size_.x = width;
    size_.y = height;
    create_swapchain_and_resources();
    for (auto& cb : resize_cbs_) {
        cb(*this, size_.x, size_.y);
    }
}

void Swapchain::on_resize(std::function<void(Swapchain&, u32, u32)> resize_cb) noexcept {
    resize_cbs_.emplace_back(std::move(resize_cb));
}

void Swapchain::reset() noexcept {
    cleanup_swapchain_and_resources();
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
}

ViewportInfo Swapchain::get_viewport_info() const noexcept {
    return { VkViewport{
                 0.0f,                        // x;
                 0.0f,                        // y;
                 static_cast<float>(size_.x), // width;
                 static_cast<float>(size_.y), // height;
                 0.0f,                        // minDepth;
                 1.0f                         // maxDepth;
             },
             VkRect2D{ VkOffset2D{ 0, 0 }, extent() } };
}

// void Swapchain::render(
//     std::function<void(render::scene::CommandBuffer& command_context, VkRenderPassBeginInfo renderpass_info)>
//         exec) noexcept {
//     // wait for last frame to finish
//     sync_objects_[current_sync_objects_index_].in_flight_fence.wait();
//     sync_objects_[current_sync_objects_index_].in_flight_fence.reset();
//
//     VkRenderPassBeginInfo renderpass_info{};
//     renderpass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//     renderpass_info.renderPass        = renderpass_;
//     renderpass_info.framebuffer       = framebuffers_[current_frame_];
//     renderpass_info.renderArea.offset = { 0, 0 };
//     renderpass_info.renderArea.extent = { static_cast<u32>(size_.x), static_cast<u32>(size_.y) };
//     renderpass_info.pNext             = nullptr;
//
//     const static VkClearValue clear_color = { { { 0.1f, 0.1f, 0.1f, 1.0f } } };
//     VkClearValue clear_depth              = {};
//     clear_depth.depthStencil.depth        = 1.0f;
//
//     VkClearValue clear_values[]     = { clear_color, clear_depth };
//     renderpass_info.clearValueCount = 2;
//     renderpass_info.pClearValues    = +clear_values;
//
//     command_buffers_[current_frame_].record([&] { exec(command_buffers_[current_frame_], renderpass_info); });
//
//     VkSemaphore wait_semaphores[]      = { sync_objects_[current_sync_objects_index_].image_avail };
//     VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//     VkSemaphore signal_semaphores[]    = { sync_objects_[current_sync_objects_index_].render_done };
//     command_buffers_[current_frame_].submit(
//         wait_semaphores,
//         wait_stages,
//         signal_semaphores,
//         sync_objects_[current_sync_objects_index_].in_flight_fence);
// }

void Swapchain::present() noexcept {
    VkSwapchainKHR swapchains[]     = { underlying_ };
    VkSemaphore signal_semaphores[] = { sync_objects_[current_sync_objects_index_].render_done };
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = swapchains;
    present_info.pImageIndices      = &current_frame_;

    auto queue      = context_.retrieve(Operation::present);
    VkResult result = vkQueuePresentKHR(queue, &present_info);

    // increment to get next sync object
    current_sync_objects_index_ = (current_sync_objects_index_ + 1) % std::size(sync_objects_);

    if (result == VK_SUCCESS)
        assure(vkAcquireNextImageKHR(
            context_,
            underlying_,
            std::numeric_limits<std::uint64_t>::max(),
            sync_objects_[current_sync_objects_index_].image_avail,
            nullptr,
            &current_frame_));
    else {
        size_ = get_new_size();
        force_resize(size_.x, size_.y);
    }
}

void Swapchain::assure(VkResult result) noexcept {
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ZOO_LOG_INFO("Should recreate");
        size_ = get_new_size();
        force_resize(size_.x, size_.y);
    } else if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS)
        ZOO_LOG_ERROR("FAILED with {}", string_VkResult(result));
}

Swapchain::FrameInfo Swapchain::frame_info() const noexcept {
    return { .current = current_image(), // current
             .count   = num_images() };
}

const resources::TextureView* Swapchain::get_image(s32 index) const noexcept {
    return index < (s32)views_.size() ? &views_[index] : nullptr;
}

s32 Swapchain::num_images() const noexcept { return static_cast<s32>(images_.size()); }

scene::PresentContext Swapchain::current_present_context() const noexcept {
    return { sync_objects_[current_sync_objects_index_].image_avail,
             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
             sync_objects_[current_sync_objects_index_].render_done };
}
// @TODO: this function should be named better.
s32 Swapchain::current_image() const noexcept { return static_cast<s32>(current_frame_); }

} // namespace zoo::render
