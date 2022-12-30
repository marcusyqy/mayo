#include "swapchain.hpp"
#include "render/fwd.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <optional>

namespace zoo::render {

namespace {
std::optional<size_t> get_queue_index_if_physical_device_is_chosen(
    const render::utils::physical_device& physical_device,
    VkSurfaceKHR surface) noexcept {
    if (!physical_device.has_geometry_shader() &&
        physical_device.has_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        return std::nullopt;

    size_t index{};
    for (const auto& queue_properties : physical_device.queue_properties()) {
        if (queue_properties.has_graphics() &&
            physical_device.has_present(queue_properties, surface))
            return std::make_optional(index);
        ++index;
    }
    return std::nullopt;
}

struct swapchain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

    swapchain_support_details& update(
        const utils::physical_device& physical_device,
        VkSurfaceKHR surface) noexcept;
};

swapchain_support_details& swapchain_support_details::update(
    const utils::physical_device& physical_device,
    VkSurfaceKHR surface) noexcept {

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface, &capabilities);

    uint32_t format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &format_count, nullptr);

    if (format_count != 0) {
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, surface, &format_count, formats.data());
    }

    uint32_t present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
            &present_mode_count, present_modes.data());
    }
    return *this;
}

bool is_device_compatible(const swapchain_support_details& details) noexcept {
    return !details.formats.empty() && !details.present_modes.empty();
}

// TODO: create a priority list to choose suface format. for now just hardcode
// this.
VkSurfaceFormatKHR choose_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats) noexcept {
    for (const auto& format : available_formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }
    return available_formats.front();
}

VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>&
        available_presentation_modes) noexcept {
    for (const auto& mode : available_presentation_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR& capabilities,
    detail::window_size_type width, detail::window_size_type height) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual_extent = {
        static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    actual_extent.width = std::clamp(actual_extent.width,
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

    actual_extent.height = std::clamp(actual_extent.height,
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual_extent;
}

} // namespace
  //
swapchain::~swapchain() noexcept {
    // we will not call resize here because swapchain will be gracefully removed
    // using the create_info struct.
    cleanup_swapchain_and_resources();
    reset();
}
//
swapchain::swapchain(const render::engine& engine,
    underlying_window_type glfw_window, width_type x, width_type y) noexcept :
    instance_(engine.vk_instance()) {
    // create surface first
    VK_EXPECT_SUCCESS(
        glfwCreateWindowSurface(instance_, glfw_window, nullptr, &surface_));

    // choose device first
    // call resize;

    platform::render::parameters params{true};
    platform::render::query query{params};

    swapchain_support_details details{};
    for (const auto& pd : engine.physical_devices()) {
        auto optional_index =
            get_queue_index_if_physical_device_is_chosen(pd, surface_);
        if (optional_index &&
            is_device_compatible(details.update(pd, surface_))) {
            context_ = std::make_shared<device_context>(
                instance_, pd, pd.queue_properties()[*optional_index], query);
            break;
        }
    }
    description_.surface_format = choose_surface_format(details.formats);
    description_.present_mode = choose_present_mode(details.present_modes);
    description_.capabilities = std::move(details.capabilities);

    // should work correctly
    // don't set  x and y so that resize can check for new setting
    resize(x, y);
}
bool swapchain::create_swapchain_and_resources() noexcept {
    VkExtent2D extent =
        choose_extent(description_.capabilities, size_.x, size_.y);

    uint32_t image_count =
        std::clamp(description_.capabilities.minImageCount + 1,
            description_.capabilities.minImageCount,
            description_.capabilities.minImageCount);

    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface_;
    create_info.minImageCount = image_count;
    create_info.imageFormat = description_.surface_format.format;
    create_info.imageColorSpace = description_.surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.preTransform = description_.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = description_.present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = underlying_;

    bool failed = false;
    VK_EXPECT_SUCCESS(
        vkCreateSwapchainKHR(*context_, &create_info, nullptr, &underlying_),
        [&failed](VkResult /* result */) { failed = true; });

    // retrieve images
    vkGetSwapchainImagesKHR(*context_, underlying_, &image_count, nullptr);
    images_.resize(image_count);
    vkGetSwapchainImagesKHR(
        *context_, underlying_, &image_count, images_.data());

    for (auto view : views_) {
        vkDestroyImageView(*context_, view, nullptr);
    }
    views_.resize(image_count);

    auto view = views_.data();
    for (const auto& image : images_) {
        VkImageViewCreateInfo view_create_info{};
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image = image;
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format = description_.surface_format.format;

        view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        view_create_info.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        view_create_info.subresourceRange.baseMipLevel = 0;
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.subresourceRange.baseArrayLayer = 0;
        view_create_info.subresourceRange.layerCount = 1;

        VK_EXPECT_SUCCESS(
            vkCreateImageView(*context_, &view_create_info, nullptr, view++),
            [&view, this, &failed](VkResult /* result */) {
                ZOO_LOG_ERROR("Failed to create image views! For index",
                    std::distance(views_.data(), view - 1));
                failed = true;
            });
    }

    return !failed;
}

void swapchain::cleanup_swapchain_and_resources() noexcept {
    vkDestroySwapchainKHR(*context_, underlying_, nullptr);

    for (auto view : views_) {
        vkDestroyImageView(*context_, view, nullptr);
    }
}

bool swapchain::resize(width_type x, width_type y) noexcept {
    if (x == size_.x && y == size_.y) {
        // don't need to resize if sizes are the same.
        return false;
    }
    size_.x = x;
    size_.y = y;

    return create_swapchain_and_resources();
}

void swapchain::reset() noexcept {
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
}

} // namespace zoo::render
