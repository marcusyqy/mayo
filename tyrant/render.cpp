#include "render.hpp"
#include "logger.hpp"
#include <assert.h>
#include <vulkan/vk_enum_string_helper.h>
// for now.
#include <vma/vk_mem_alloc.h>

#ifdef WIN32
#include <vulkan/vulkan_win32.h>
#undef max
#endif

#include "basic.hpp"
#include "shader_compiler.hpp"
#include <algorithm>
#include <limits>
#include <string_view>

#define ENABLE_VALIDATION 1

// forward declare
Swapchain create_swapchain_from_surface(VkSurfaceKHR surface);
void recreate_swapchain(Swapchain& swapchain);
void release_resize();

namespace {

struct Resize_Stuff {
    VkFence fence             = {};
    VkImageView image_view    = {};
    VkSemaphore semaphores[2] = {};
    VkFramebuffer framebuffer = {};
    bool used                 = false;
};

Resize_Stuff resize_stuffs[10000];
size_t num_resize_stuff;

constexpr auto API_VERSION                  = VK_API_VERSION_1_3;
constexpr const char* VALIDATION_LAYER      = "VK_LAYER_KHRONOS_validation";
constexpr const char* required_extensions[] = {
    "VK_KHR_surface",
#ifdef WIN32
    "VK_KHR_win32_surface",
#endif
#if ENABLE_VALIDATION
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

VkInstance instance                                         = { VK_NULL_HANDLE };
VkDebugUtilsMessengerEXT debug_messenger                    = { VK_NULL_HANDLE };
VkDebugReportCallbackEXT debug_report                       = { VK_NULL_HANDLE };
VkPhysicalDevice devices[Render_Params::MAX_NUMBER_DEVICES] = {};

struct Device {
    VkDevice logical          = { VK_NULL_HANDLE };
    VkPhysicalDevice physical = { VK_NULL_HANDLE };
    VkQueue present_queue;
    VkQueue graphics_queue;
    VkCommandPool command_pool;
} gpu = {};

u32 device_count = {};

void maybe_invoke(VkResult result) noexcept { assert(result == VK_SUCCESS); }

template <typename Call, typename... Args>
void maybe_invoke(VkResult result, Call then, Args&&... args) noexcept {
    then(result, std::forward<Args>(args)...);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT object_type,
    std::uint64_t object,
    std::size_t location,
    std::int32_t message_code,
    const char* layer_prefix,
    const char* message,
    void* user_data) {
    log_error(message);
    return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    [[maybe_unused]] void* user_data) noexcept {

    const char* prepend = nullptr;
    if (type >= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        prepend = "PERFORMANCE";
    } else if (type >= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        prepend = "VALIDATION";
    } else if (type >= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        prepend = "GENERAL";
    }

    static constexpr char validation_message[] = "<{}> :|vulkan|: {}";
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        log_error(validation_message, prepend, callback_data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log_warn(validation_message, prepend, callback_data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        log_info(validation_message, prepend, callback_data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        log_trace(validation_message, prepend, callback_data->pMessage);
    }
    return VK_FALSE;
}

} // namespace

#define VK_EXPECT_SUCCESS(EXP, ...)                                                                                    \
    if (VkResult ____result = EXP; ____result != VK_SUCCESS) {                                                         \
        log_error("VK_SUCCESS NOT MET FOR CALL : " #EXP " , failed with exit code = {}", string_VkResult(____result)); \
        maybe_invoke(____result, __VA_ARGS__);                                                                         \
    }

void init_vulkan_resources() {
    // create_instance
    {
        VkApplicationInfo app_info{ .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                    .pNext              = nullptr, // for now
                                    .pApplicationName   = "Zoo Engine",
                                    .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
                                    .pEngineName        = "Zoo Engine",
                                    .engineVersion      = VK_MAKE_VERSION(0, 0, 0),
                                    .apiVersion         = API_VERSION };

        Buffer_View<const char* const> layers = { nullptr, 0 };
#if ENABLE_VALIDATION
        u32 layer_count = {};
        VkLayerProperties available_layers[100];
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        assert(layer_count < 100);
        vkEnumerateInstanceLayerProperties(&layer_count, +available_layers);

        for (u32 i = 0; i < layer_count; ++i) {
            if (!strcmp(VALIDATION_LAYER, available_layers[i].layerName)) {
                layers = { &VALIDATION_LAYER, 1 };
                break;
            }
        }
#endif

        VkInstanceCreateInfo create_info{
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .pApplicationInfo        = &app_info,
            .enabledLayerCount       = (u32)layers.count,
            .ppEnabledLayerNames     = layers.data,
            .enabledExtensionCount   = ARRAY_SIZE(required_extensions),
            .ppEnabledExtensionNames = required_extensions,
        };

        instance = VK_NULL_HANDLE;
        VK_EXPECT_SUCCESS(vkCreateInstance(&create_info, nullptr, &instance));
    }

    // create_debug_messenger
#if ENABLE_VALIDATION
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info = {};
        {
            create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = debug_callback;
            create_info.pUserData       = nullptr; // Optional
        }

        auto vkCreateDebugUtilsMessengerEXT =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        assert(vkCreateDebugUtilsMessengerEXT);

        VK_EXPECT_SUCCESS(vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger));

        VkDebugReportCallbackCreateInfoEXT debug_report_callback_info = {};
        debug_report_callback_info.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_callback_info.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debug_report_callback_info.pfnCallback = (PFN_vkDebugReportCallbackEXT)debug_report_callback;

        // We have to explicitly load this function.
        auto vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        assert(vkCreateDebugReportCallbackEXT);

        VK_EXPECT_SUCCESS(
            vkCreateDebugReportCallbackEXT(instance, &debug_report_callback_info, nullptr, &debug_report));
    }
#endif // ENABLE_VALIDATION

    // for some reason we have to get the count first then get the devices?
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
    assert(device_count != 0);
    assert(device_count < Render_Params::MAX_NUMBER_DEVICES);
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, +devices));

    // select and create device if not already created. @TODO: maybe we need a mutex here.
    u32 most_compatible                = 0;
    u32 most_compatible_graphics_queue = 0;
    u32 most_compatible_present_queue  = 0;

    for (u32 i = 0; i < device_count; ++i) {
        const auto& physical_device                                                 = devices[i];
        VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_feature = {};
        shader_draw_parameters_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        shader_draw_parameters_feature.pNext = nullptr;
        shader_draw_parameters_feature.shaderDrawParameters = VK_TRUE;

        VkPhysicalDeviceFeatures2 features;
        features.pNext = &shader_draw_parameters_feature;
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        vkGetPhysicalDeviceFeatures2(physical_device, &features);
        if (shader_draw_parameters_feature.shaderDrawParameters != VK_TRUE) continue;

        if (features.features.geometryShader != VK_TRUE) continue;

        constexpr u32 MAX_EXTENSIONS = 300;
        u32 extension_count{};
        VkExtensionProperties available_extensions[MAX_EXTENSIONS];
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
        assert(extension_count < MAX_EXTENSIONS);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions);

        u32 j = 0;
        for (; j < extension_count; ++j) {
            auto& props = available_extensions[j];
            if (!strcmp(props.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) break;
        }

        // did not find.
        if (j == extension_count) continue;

        u32 queue_family_count        = 0;
        constexpr u32 MAX_QUEUE_COUNT = 20;
        VkQueueFamilyProperties queue_families[MAX_QUEUE_COUNT];
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
        assert(queue_family_count < MAX_QUEUE_COUNT);

        bool found_graphics = false;
        bool found_present  = false;

        u32 graphics_queue_idx = 0;
        u32 present_queue_idx  = 0;
        for (j = 0; j < queue_family_count; ++j) {
            // @NOTE: assume graphics supports all types of queue.
            if (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                found_graphics     = true;
                graphics_queue_idx = j;
            }
#ifdef WIN32
            VkBool32 presentation_support = vkGetPhysicalDeviceWin32PresentationSupportKHR(physical_device, j);
#else
            static_assert(false); // TODO: this needs to be changed
            VkBool32 presentation_support = VK_FALSE;
#endif
            // vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, j, surface, &presentation_support);
            if (presentation_support) {
                found_present     = true;
                present_queue_idx = j;
            }

            if (found_present && found_graphics) break;
        }

        if (!found_present || !found_graphics) continue;

        most_compatible                = i;
        most_compatible_graphics_queue = graphics_queue_idx;
        most_compatible_present_queue  = present_queue_idx;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) break;
    }

    const auto& pd                = devices[most_compatible];
    constexpr u32 MAX_QUEUE_COUNT = 20;
    u32 queue_family_count        = 0;
    VkQueueFamilyProperties queue_families[MAX_QUEUE_COUNT];
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_count, nullptr);
    assert(queue_family_count < MAX_QUEUE_COUNT);
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_count, queue_families);

    float queue_priority                         = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info[2] = {};

    queue_create_info[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[0].queueFamilyIndex = most_compatible_graphics_queue;
    queue_create_info[0].pQueuePriorities = &queue_priority;
    queue_create_info[0].queueCount       = 1;

    queue_create_info[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[1].queueFamilyIndex = most_compatible_present_queue;
    queue_create_info[1].pQueuePriorities = &queue_priority;
    queue_create_info[1].queueCount       = 1;

    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_feature = {};
    shader_draw_parameters_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shader_draw_parameters_feature.pNext = nullptr;
    shader_draw_parameters_feature.shaderDrawParameters = VK_TRUE;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(pd, &features);

    const char* device_extension{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo create_info{};
    create_info.pNext                   = &shader_draw_parameters_feature;
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount    = most_compatible_present_queue == most_compatible_graphics_queue ? 1 : 2;
    create_info.pQueueCreateInfos       = queue_create_info;
    create_info.pEnabledFeatures        = &features;
    create_info.ppEnabledExtensionNames = &device_extension;
    create_info.enabledExtensionCount   = 1;

    VkDevice device = { VK_NULL_HANDLE };
    VK_EXPECT_SUCCESS(vkCreateDevice(pd, &create_info, nullptr, &device));

    VkQueue present_queue  = { VK_NULL_HANDLE };
    VkQueue graphics_queue = { VK_NULL_HANDLE };
    vkGetDeviceQueue(device, most_compatible_graphics_queue, 0, &graphics_queue);
    vkGetDeviceQueue(device, most_compatible_present_queue, 0, &present_queue);

    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_create_info.queueFamilyIndex = most_compatible_graphics_queue;

    VkCommandPool command_pool = { VK_NULL_HANDLE };
    VK_EXPECT_SUCCESS(vkCreateCommandPool(device, &pool_create_info, nullptr, &command_pool));
    // we need to handle this case
    assert(most_compatible_graphics_queue == most_compatible_present_queue);

    gpu = { .logical        = device,
            .physical       = pd,
            .present_queue  = present_queue,
            .graphics_queue = graphics_queue,
            .command_pool   = command_pool };
}

void free_vulkan_resources() {
    if (gpu.logical) {
        vkDestroyCommandPool(gpu.logical, gpu.command_pool, nullptr);
        vkDestroyDevice(gpu.logical, nullptr);
    }

#if !ENABLE_VALIDATION
    auto vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    assert(vkDestroyDebugUtilsMessengerEXT);
    vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);

    auto vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    assert(vkDestroyDebugReportCallbackEXT);
    vkDestroyDebugReportCallbackEXT(instance, debug_report, nullptr);
#endif // ENABLE_VALIDATION

    vkDestroyInstance(instance, nullptr);
}

Swapchain create_swapchain_from_surface(VkSurfaceKHR surface) {
    Swapchain swapchain = {};
    swapchain.surface   = surface;

    // create_swapchain here.
    recreate_swapchain(swapchain);

    return swapchain;
}

void recreate_swapchain(Swapchain& swapchain) {
    // @TODO: erase all resources without waiting for gpu to be idle
    vkDeviceWaitIdle(gpu.logical);

    const auto old_num_images             = swapchain.num_images;
    const auto& surface                   = swapchain.surface;
    VkSurfaceCapabilitiesKHR capabilities = {};
    VkSurfaceFormatKHR formats[Render_Params::MAX_FORMAT_COUNT];
    VkPresentModeKHR present_modes[Render_Params::MAX_PRESENT_MODE_COUNT];

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.physical, surface, &capabilities);

    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physical, surface, &format_count, nullptr);
    assert(format_count != 0);
    assert(format_count < Render_Params::MAX_FORMAT_COUNT);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physical, surface, &format_count, formats);

    u32 present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.physical, surface, &present_mode_count, nullptr);
    assert(present_mode_count < Render_Params::MAX_PRESENT_MODE_COUNT);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.physical, surface, &present_mode_count, present_modes);
    //

    bool format_chosen = false;
    for (u32 i = 0; i < format_count; ++i) {
        const auto& format = formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain.format = format;
            format_chosen    = true;
            break;
        }
    }

    if (!format_chosen) swapchain.format = formats[0];

    VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < present_mode_count; ++i) {
        const auto& mode = present_modes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) chosen_present_mode = mode;
    }

    if (capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != UINT32_MAX) {
        swapchain.width  = capabilities.currentExtent.width;
        swapchain.height = capabilities.currentExtent.height;
    } else {
        VkExtent2D actual_extent = { static_cast<u32>(swapchain.width), static_cast<u32>(swapchain.height) };
        actual_extent.width =
            std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

        actual_extent.height =
            std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        swapchain.width  = actual_extent.width;
        swapchain.height = actual_extent.height;
    }

    u32 image_count =
        std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR create_info{ .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                          .pNext                 = nullptr,
                                          .flags                 = 0,
                                          .surface               = surface,
                                          .minImageCount         = image_count,
                                          .imageFormat           = swapchain.format.format,
                                          .imageColorSpace       = swapchain.format.colorSpace,
                                          .imageExtent           = { swapchain.width, swapchain.height },
                                          .imageArrayLayers      = 1,
                                          .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                          .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
                                          .queueFamilyIndexCount = 0,
                                          .pQueueFamilyIndices   = nullptr,
                                          .preTransform          = capabilities.currentTransform,
                                          .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                          .presentMode           = chosen_present_mode,
                                          .clipped               = VK_TRUE,
                                          .oldSwapchain          = swapchain.handle };

    VK_EXPECT_SUCCESS(vkCreateSwapchainKHR(gpu.logical, &create_info, nullptr, &swapchain.handle));

    // cleanup.
    for (u32 i = 0; i < old_num_images; ++i) {
        vkDestroyImageView(gpu.logical, swapchain.image_views[i], nullptr);
        vkDestroyFence(gpu.logical, swapchain.in_flight[i], nullptr);
        vkDestroySemaphore(gpu.logical, swapchain.render_done[i], nullptr);
        vkDestroySemaphore(gpu.logical, swapchain.image_avail[i], nullptr);
    }

    // required to safely destroy after creating new swapchain.
    if (create_info.oldSwapchain != nullptr) vkDestroySwapchainKHR(gpu.logical, create_info.oldSwapchain, nullptr);

    // retrieve images
    VkImage placeholder[Render_Params::MAX_SWAPCHAIN_IMAGES];
    vkGetSwapchainImagesKHR(gpu.logical, swapchain.handle, &swapchain.num_images, nullptr);
    assert(swapchain.num_images <= Render_Params::MAX_SWAPCHAIN_IMAGES);
    vkGetSwapchainImagesKHR(gpu.logical, swapchain.handle, &swapchain.num_images, placeholder);

    for (u32 i = 0; i < swapchain.num_images; ++i) {
        const auto& image = placeholder[i];

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image    = image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format   = swapchain.format.format;

        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel   = 0;
        image_view_create_info.subresourceRange.levelCount     = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount     = 1;

        // create image view
        VK_EXPECT_SUCCESS(
            vkCreateImageView(gpu.logical, &image_view_create_info, nullptr, swapchain.image_views + i),
            [i](VkResult result) {
                log_error("Failed to create image views! For index {}", i);
                assert(result == VK_SUCCESS);
            });

        // create semaphore
        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VK_EXPECT_SUCCESS(vkCreateSemaphore(gpu.logical, &semaphore_info, nullptr, swapchain.image_avail + i));
        VK_EXPECT_SUCCESS(vkCreateSemaphore(gpu.logical, &semaphore_info, nullptr, swapchain.render_done + i));

        // create fence (maybe i don't need this)
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // TODO: figure out if we really need to signal at the start.
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_EXPECT_SUCCESS(vkCreateFence(gpu.logical, &fence_info, nullptr, swapchain.in_flight + i));
    }

    swapchain.out_of_date = false;

    VkResult result = vkAcquireNextImageKHR(
        gpu.logical,
        swapchain.handle,
        std::numeric_limits<std::uint64_t>::max(),
        swapchain.image_avail[swapchain.out_of_sync_index],
        nullptr,
        &swapchain.current_frame);

    assert(result == VK_SUBOPTIMAL_KHR || result == VK_SUCCESS);
    if (result == VK_SUBOPTIMAL_KHR) swapchain.out_of_date = true;
}

void free_swapchain(Swapchain& swapchain) {
    vkDeviceWaitIdle(gpu.logical);
    for (u32 i = 0; i < swapchain.num_images; ++i) {
        vkDestroyImageView(gpu.logical, swapchain.image_views[i], nullptr);
        vkDestroyFence(gpu.logical, swapchain.in_flight[i], nullptr);
        vkDestroySemaphore(gpu.logical, swapchain.render_done[i], nullptr);
        vkDestroySemaphore(gpu.logical, swapchain.image_avail[i], nullptr);
    }

    if (swapchain.handle) {
        vkDestroySwapchainKHR(gpu.logical, swapchain.handle, nullptr);
    }
    if (swapchain.surface) {
        vkDestroySurfaceKHR(instance, swapchain.surface, nullptr);
    }
}

void resize_swapchain(Swapchain& swapchain, u32 width, u32 height) {
    swapchain.width  = width;
    swapchain.height = height;
    recreate_swapchain(swapchain);
}

void present_swapchain(Swapchain& swapchain) {
    VkSwapchainKHR swapchains[]     = { swapchain.handle };
    VkSemaphore signal_semaphores[] = { swapchain.render_done[swapchain.out_of_sync_index] };
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = swapchains;
    present_info.pImageIndices      = &swapchain.current_frame;

    // get queue from device.
    VkResult result = vkQueuePresentKHR(gpu.present_queue, &present_info);

    // increment to get next sync object
    swapchain.out_of_sync_index = (swapchain.out_of_sync_index + 1) % swapchain.num_images;

    if (result == VK_SUCCESS) {
        result = vkAcquireNextImageKHR(
            gpu.logical,
            swapchain.handle,
            std::numeric_limits<std::uint64_t>::max(),
            swapchain.image_avail[swapchain.out_of_sync_index],
            nullptr,
            &swapchain.current_frame);
        assert(result == VK_SUBOPTIMAL_KHR || result == VK_SUCCESS);

        if (result == VK_SUBOPTIMAL_KHR) swapchain.out_of_date = true;
    } else
        swapchain.out_of_date = true;
}

VkShaderModule create_shader(Buffer_View<const u32> buffer) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize  = buffer.count;
    create_info.pCode     = buffer.data;
    VkShaderModule module = { VK_NULL_HANDLE };
    VK_EXPECT_SUCCESS(vkCreateShaderModule(gpu.logical, &create_info, nullptr, &module));
    return module;
}

void free_shader(VkShaderModule shader) {
    if (shader) vkDestroyShaderModule(gpu.logical, shader, nullptr);
}

#ifdef WIN32

Swapchain create_swapchain_from_win32(HINSTANCE hinstance, HWND hwnd) {
    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hinstance                   = hinstance;
    create_info.hwnd                        = hwnd;
    VkSurfaceKHR surface                    = { VK_NULL_HANDLE };
    VK_EXPECT_SUCCESS(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));
    return create_swapchain_from_surface(surface);
}

#endif

// TEMP FILE
#include <fstream>

namespace {

struct Pipeline {
    VkRenderPass renderpass = { VK_NULL_HANDLE };
    VkPipelineLayout layout = { VK_NULL_HANDLE };
    VkPipeline handle       = { VK_NULL_HANDLE };
} pipeline = {};

struct Framebuffer_Info {
    // assume always same swapchain for now. @TODO: this needs to be changed to make it more robust later on.
    VkImageView image_views[Render_Params::MAX_SWAPCHAIN_IMAGES] = {};
    VkFramebuffer handles[Render_Params::MAX_SWAPCHAIN_IMAGES]   = {};
    u32 width[Render_Params::MAX_SWAPCHAIN_IMAGES]               = {};
    u32 height[Render_Params::MAX_SWAPCHAIN_IMAGES]              = {};
};

} // namespace

struct Draw_Data {
    VkDescriptorPool pool      = { VK_NULL_HANDLE };
    VkDescriptorSet static_set = { VK_NULL_HANDLE };

    VkFence fences[Render_Params::MAX_SWAPCHAIN_IMAGES]                  = {};
    VkCommandBuffer command_buffers[Render_Params::MAX_SWAPCHAIN_IMAGES] = {};
    VkDescriptorSet dynamic_sets[Render_Params::MAX_SWAPCHAIN_IMAGES]    = {};
    Framebuffer_Info framebuffer                                         = {};
};

// @TODO : make shader system more robust?
void create_shaders_and_pipeline() {
    const auto read_file = [](std::string_view filename) -> Buffer_View<char> {
        std::ifstream file{ filename.data(), std::ios::ate | std::ios::binary };
        assert(file.is_open());

        const size_t file_size   = (size_t)file.tellg();
        Buffer_View<char> buffer = { new char[file_size], file_size };
        file.seekg(0);
        file.read(buffer.data, buffer.count);
        file.close();
        return buffer;
    };

    auto vert_buffer = read_file("shaders/color.vert");
    defer { delete[] vert_buffer.data; };

    auto frag_buffer = read_file("shaders/color.frag");
    defer { delete[] frag_buffer.data; };

    Shader_Compiler shader_compiler = create_shader_compiler();
    defer { free_shader_compiler(shader_compiler); };

    Shader_Work vert_work = { .kind  = shaderc_vertex_shader,
                              .name  = "color.vert",
                              .bytes = { vert_buffer.data, vert_buffer.count } };

    Shader_Work frag_work = { .kind  = shaderc_fragment_shader,
                              .name  = "color.frag",
                              .bytes = { frag_buffer.data, frag_buffer.count } };

    auto spv_vert = shader_compiler.compile(vert_work);
    defer { shader_compiler.free(spv_vert); };
    auto spv_frag = shader_compiler.compile(frag_work);
    defer { shader_compiler.free(spv_frag); };

    auto vert_shader = create_shader({ spv_vert.data(), spv_vert.size_in_bytes() });
    defer { free_shader(vert_shader); };

    auto frag_shader = create_shader({ spv_frag.data(), spv_frag.size_in_bytes() });
    defer { free_shader(frag_shader); };

    // create pipeline
    enum : uint32_t { vertex_stage = 0, fragment_stage = 1, shader_stages = 2 };
    VkPipelineShaderStageCreateInfo shaders_create_info[shader_stages]{};
    {
        VkPipelineShaderStageCreateInfo& vertex_create_info{ shaders_create_info[vertex_stage] };
        vertex_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_create_info.module = vert_shader;
        vertex_create_info.pName  = "main";
    }
    {
        VkPipelineShaderStageCreateInfo& fragment_create_info{ shaders_create_info[fragment_stage] };
        fragment_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_create_info.module = frag_shader;
        fragment_create_info.pName  = "main";
    }

    VkDynamicState dynamic_states_array[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = (u32)ARRAY_SIZE(dynamic_states_array);
    dynamic_state.pDynamicStates    = +dynamic_states_array;

    VkViewport viewport = {};
    VkRect2D scissor    = {};

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports    = &viewport;
    viewport_state_create_info.scissorCount  = 1;
    viewport_state_create_info.pScissors     = &scissor;

    // INPUT ASSEMBLY
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    // RASTERIZATION
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
    rasterizer_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_create_info.depthClampEnable        = VK_FALSE;
    rasterizer_create_info.depthBiasEnable         = VK_FALSE;
    rasterizer_create_info.depthBiasConstantFactor = 0.0f;              // Optional
    rasterizer_create_info.depthBiasClamp          = 0.0f;              // Optional
    rasterizer_create_info.depthBiasSlopeFactor    = 0.0f;              // Optional
    rasterizer_create_info.cullMode                = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
    rasterizer_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_create_info.lineWidth               = 1.0f;
    rasterizer_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling_create_info{};
    multisampling_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_create_info.sampleShadingEnable   = VK_FALSE;
    multisampling_create_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling_create_info.minSampleShading      = 1.0f;     // Optional
    multisampling_create_info.pSampleMask           = nullptr;  // Optional
    multisampling_create_info.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling_create_info.alphaToOneEnable      = VK_FALSE; // Optional

    VkAttachmentDescription color_attachment{};
    color_attachment.format         = VK_FORMAT_B8G8R8A8_SRGB; // try @TODO: make format correct.
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_attachment_ref;
    subpass.pDepthStencilAttachment = nullptr; //&depth_attachment_ref;

    VkRenderPassCreateInfo renderpass_info{};
    renderpass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_info.attachmentCount = 1;
    renderpass_info.pAttachments    = &color_attachment;
    renderpass_info.subpassCount    = 1;
    renderpass_info.pSubpasses      = &subpass;
    renderpass_info.dependencyCount = 1;
    renderpass_info.pDependencies   = &dependency;

    VK_EXPECT_SUCCESS(vkCreateRenderPass(gpu.logical, &renderpass_info, nullptr, &pipeline.renderpass));

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount         = 0;
    pipeline_layout_create_info.pSetLayouts            = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges    = nullptr;

    VK_EXPECT_SUCCESS(vkCreatePipelineLayout(gpu.logical, &pipeline_layout_create_info, nullptr, &pipeline.layout));

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    color_blend_attachment_state.blendEnable                         = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor                 = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor                 = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp                        = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp                        = VK_BLEND_OP_ADD;
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable     = VK_FALSE;
    color_blend_state_create_info.logicOp           = VK_LOGIC_OP_COPY; // Optional
    color_blend_state_create_info.attachmentCount   = 1;
    color_blend_state_create_info.pAttachments      = &color_blend_attachment_state;
    color_blend_state_create_info.blendConstants[0] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[1] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[2] = 0.0f; // Optional
    color_blend_state_create_info.blendConstants[3] = 0.0f; // Optional

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount   = 0;
    vertex_input_state_create_info.pVertexBindingDescriptions      = nullptr;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_state_create_info.pVertexAttributeDescriptions    = nullptr;

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.stageCount          = ARRAY_SIZE(shaders_create_info);
    graphics_pipeline_create_info.pStages             = +shaders_create_info;
    graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterizer_create_info;
    graphics_pipeline_create_info.pMultisampleState   = &multisampling_create_info;
    graphics_pipeline_create_info.pDepthStencilState  = nullptr; // &depth_stencil_state_info; // Optional
    graphics_pipeline_create_info.pColorBlendState    = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState       = &dynamic_state;
    graphics_pipeline_create_info.layout              = pipeline.layout;
    graphics_pipeline_create_info.renderPass          = pipeline.renderpass;
    graphics_pipeline_create_info.subpass             = 0;
    graphics_pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE; // Optional
    graphics_pipeline_create_info.basePipelineIndex   = -1;             // Optional

    VK_EXPECT_SUCCESS(
        vkCreateGraphicsPipelines(gpu.logical, nullptr, 1, &graphics_pipeline_create_info, nullptr, &pipeline.handle));
}

void draw(Swapchain& swapchain, Draw_Data* draw_data) {
    assert(draw_data);

    vkWaitForFences(
        gpu.logical,
        1,
        draw_data->fences + swapchain.current_frame,
        VK_TRUE,
        std::numeric_limits<u64>::max());

    vkResetFences(gpu.logical, 1, draw_data->fences + swapchain.current_frame);
    auto& framebuffer = draw_data->framebuffer;

    // if (swapchain.width != framebuffer.width[swapchain.current_frame] ||
    //     swapchain.height != framebuffer.height[swapchain.current_frame]) {
    if (framebuffer.image_views[swapchain.current_frame] != swapchain.image_views[swapchain.current_frame]) {
        framebuffer.width[swapchain.current_frame]  = swapchain.width;
        framebuffer.height[swapchain.current_frame] = swapchain.height;

        // recreate_framebuffer
        auto& current_framebuffer = framebuffer.handles[swapchain.current_frame];
        if (current_framebuffer) {
            // bool slot_filled = false;
            // for (size_t i = 0; i < num_resize_stuff; ++i) {
            //     if (resize_stuffs[i].used &&
            //         framebuffer.image_views[swapchain.current_frame] == resize_stuffs[i].image_view) {
            //         resize_stuffs[i].framebuffer = current_framebuffer;
            //         slot_filled                  = true;
            //     }
            // }
            // assert(slot_filled);
            vkDestroyFramebuffer(gpu.logical, current_framebuffer, nullptr);
        }

        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass      = pipeline.renderpass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments    = &swapchain.image_views[swapchain.current_frame];
        framebuffer_create_info.width           = swapchain.width;
        framebuffer_create_info.height          = swapchain.height;
        framebuffer_create_info.layers          = 1;

        framebuffer.image_views[swapchain.current_frame] = swapchain.image_views[swapchain.current_frame];

        VK_EXPECT_SUCCESS(vkCreateFramebuffer(gpu.logical, &framebuffer_create_info, nullptr, &current_framebuffer));
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags            = 0; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    vkResetCommandBuffer(draw_data->command_buffers[swapchain.current_frame], 0);
    VK_EXPECT_SUCCESS(vkBeginCommandBuffer(draw_data->command_buffers[swapchain.current_frame], &begin_info));

    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass = pipeline.renderpass;
    // @TODO: make this work.

    renderpass_info.framebuffer       = framebuffer.handles[swapchain.current_frame];
    renderpass_info.renderArea.offset = { 0, 0 };
    renderpass_info.renderArea.extent = { swapchain.width, swapchain.height };
    renderpass_info.pNext             = nullptr;

    VkClearValue clear_color;
    renderpass_info.clearValueCount = 1;
    renderpass_info.pClearValues    = &clear_color;

    VkViewport viewport = { .x        = 0.0f,
                            .y        = 0.0f,
                            .width    = (f32)swapchain.width,
                            .height   = (f32)swapchain.height,
                            .minDepth = 0.0f,
                            .maxDepth = 1.0f };

    vkCmdSetViewport(draw_data->command_buffers[swapchain.current_frame], 0, 1, &viewport);

    VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = { (u32)swapchain.width, (u32)swapchain.height },
    };
    vkCmdSetScissor(draw_data->command_buffers[swapchain.current_frame], 0, 1, &scissor);

    vkCmdBeginRenderPass(
        draw_data->command_buffers[swapchain.current_frame],
        &renderpass_info,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        draw_data->command_buffers[swapchain.current_frame],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.handle);

    vkCmdDraw(draw_data->command_buffers[swapchain.current_frame], 3, 1, 0, 0);

    vkCmdEndRenderPass(draw_data->command_buffers[swapchain.current_frame]);
    VK_EXPECT_SUCCESS(
        vkEndCommandBuffer(draw_data->command_buffers[swapchain.current_frame]),
        [](VkResult /* result */) {});

    VkPipelineStageFlags pipeline_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = swapchain.image_avail + swapchain.out_of_sync_index;
    submit_info.pWaitDstStageMask    = &pipeline_stages;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = draw_data->command_buffers + swapchain.current_frame;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = swapchain.render_done + swapchain.out_of_sync_index;

    VK_EXPECT_SUCCESS(vkQueueSubmit(gpu.graphics_queue, 1, &submit_info, draw_data->fences[swapchain.current_frame]));
}

void assert_format(VkFormat format) { assert(format == VK_FORMAT_B8G8R8A8_SRGB); }

Draw_Data* create_draw_data() {
    Draw_Data* draw_data     = new Draw_Data;
    constexpr auto pool_size = 1000;

    // clang-format off
    VkDescriptorPoolSize sizes[] {
        { VK_DESCRIPTOR_TYPE_SAMPLER, pool_size },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pool_size },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, pool_size },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, pool_size },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pool_size }
    };
    // clang-format on

    u32 count = ARRAY_SIZE(sizes);

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        // TODO: we can remove this if needed. This is needed for solving the bug above.
        .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets       = pool_size * count,
        .poolSizeCount = count,
        .pPoolSizes    = +sizes,
    };

    VK_EXPECT_SUCCESS(vkCreateDescriptorPool(gpu.logical, &pool_info, nullptr, &draw_data->pool));

    for (u32 i = 0, size = ARRAY_SIZE(draw_data->fences); i < size; ++i) {
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        // TODO: figure out if we really need to signal at the start.
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_EXPECT_SUCCESS(vkCreateFence(gpu.logical, &fence_info, nullptr, draw_data->fences + i));
    }

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = gpu.command_pool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = ARRAY_SIZE(draw_data->command_buffers);
    VK_EXPECT_SUCCESS(vkAllocateCommandBuffers(gpu.logical, &alloc_info, draw_data->command_buffers));
    return draw_data;
}

void free_draw_data(Draw_Data* draw_data) {
    assert(draw_data);
    vkFreeDescriptorSets(gpu.logical, draw_data->pool, 1, &draw_data->static_set);
    vkFreeCommandBuffers(
        gpu.logical,
        gpu.command_pool,
        ARRAY_SIZE(draw_data->command_buffers),
        draw_data->command_buffers);

    vkFreeDescriptorSets(gpu.logical, draw_data->pool, ARRAY_SIZE(draw_data->dynamic_sets), draw_data->dynamic_sets);

    for (u32 i = 0, size = ARRAY_SIZE(draw_data->fences); i < size; ++i)
        vkDestroyFence(gpu.logical, draw_data->fences[i], nullptr);
}

void free_shaders_and_pipeline() {
    vkDestroyPipelineLayout(gpu.logical, pipeline.layout, nullptr);
    vkDestroyRenderPass(gpu.logical, pipeline.renderpass, nullptr);
    vkDestroyPipeline(gpu.logical, pipeline.handle, nullptr);
}

void release_resize() {
    for (auto i = 0u; i < num_resize_stuff; ++i) {
        if (!resize_stuffs[i].used) {
            auto status = vkGetFenceStatus(gpu.logical, resize_stuffs[i].fence);
            if (VK_SUCCESS == status) {
                vkDestroyImageView(gpu.logical, resize_stuffs[i].image_view, nullptr);
                vkDestroyFence(gpu.logical, resize_stuffs[i].fence, nullptr);
                vkDestroySemaphore(gpu.logical, resize_stuffs[i].semaphores[0], nullptr);
                vkDestroySemaphore(gpu.logical, resize_stuffs[i].semaphores[1], nullptr);
                vkDestroyFramebuffer(gpu.logical, resize_stuffs[i].framebuffer, nullptr);
                resize_stuffs[i].used = true;
            }
        }
    }
}
