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

#include <limits>

#define ENABLE_VALIDATION 1

// forward declare
Swapchain create_swapchain_from_surface(VkSurfaceKHR surface);
void recreate_swapchain(Swapchain&);

namespace {

constexpr auto API_VERSION                  = VK_API_VERSION_1_3;
constexpr const char* VALIDATION_LAYER      = "VK_LAYER_KHRONOS_validation";
constexpr const char* required_extensions[] = {
    "VK_KHR_surface",
#ifdef WIN32
    "VK_KHR_win32_surface",
#endif
#if ENABLE_VALIDATION
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

enum : u32 { MAX_IMAGES = 3, MAX_NUMBER_DEVICES = 10, MAX_FORMAT_COUNT = 100, MAX_PRESENT_MODE_COUNT = 100 };

VkInstance instance                          = { VK_NULL_HANDLE };
VkDebugUtilsMessengerEXT debug_messenger     = { VK_NULL_HANDLE };
VkPhysicalDevice devices[MAX_NUMBER_DEVICES] = {};

struct Device {
    VkDevice logical          = { VK_NULL_HANDLE };
    VkPhysicalDevice physical = { VK_NULL_HANDLE };
} gpu = {};

u32 device_count = {};

void maybe_invoke(VkResult result) noexcept { assert(result == VK_SUCCESS); }

template <typename Call, typename... Args>
void maybe_invoke(VkResult result, Call then, Args&&... args) noexcept {
    then(result, std::forward<Args>(args)...);
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
        vkEnumerateInstanceLayerProperties(&layer_count, +available_layers);

        for (u32 i = 0; i < layer_count; ++i) {
            if (strcmp(VALIDATION_LAYER, available_layers[i].layerName)) {
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
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info = {};
        {
            create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = debug_callback;
            create_info.pUserData       = nullptr; // Optional
        }

        auto func =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (!func) log_warn("Debug utils messenger failed with {}", string_VkResult(VK_ERROR_EXTENSION_NOT_PRESENT));

        VkResult result = func(instance, &create_info, nullptr, &debug_messenger);
        if (result != VK_SUCCESS) {
            log_warn("Debug utils messenger failed with {}", string_VkResult(result));
            debug_messenger = VK_NULL_HANDLE;
        }
    }

    // for some reason we have to get the count first then get the devices?
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
    assert(device_count != 0);
    assert(device_count < MAX_NUMBER_DEVICES);
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, +devices));

    // probably done until we get a window?
}

void free_vulkan_resources() {
    if (debug_messenger != VK_NULL_HANDLE) {
        auto func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) func(instance, debug_messenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
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

Swapchain create_swapchain_from_surface(VkSurfaceKHR surface) {
    Swapchain swapchain = {};
    swapchain.surface   = surface;

    // select and create device if not already created. @TODO: maybe we need a mutex here.
    if (!gpu.logical) {
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

                VkBool32 presentation_support = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, j, surface, &presentation_support);
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

#if ENABLE_VALIDATION
        create_info.enabledLayerCount   = 1;
        create_info.ppEnabledLayerNames = &VALIDATION_LAYER;
#else
        create_info.enabledLayerCount   = 0;
        create_info.ppEnabledLayerNames = nullptr;
#endif

        VkDevice device;
        VK_EXPECT_SUCCESS(vkCreateDevice(pd, &create_info, nullptr, &device));
        gpu = { .logical = device, .physical = pd };
    }

    // create_swapchain here.
    recreate_swapchain(swapchain);

    return swapchain;
}

void recreate_swapchain(Swapchain& swapchain) {
    const auto& surface                   = swapchain.surface;
    VkSurfaceCapabilitiesKHR capabilities = {};
    VkSurfaceFormatKHR formats[MAX_FORMAT_COUNT];
    VkPresentModeKHR present_modes[MAX_PRESENT_MODE_COUNT];

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.physical, surface, &capabilities);

    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physical, surface, &format_count, nullptr);
    assert(format_count != 0);
    assert(format_count < MAX_FORMAT_COUNT);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physical, surface, &format_count, formats);

    u32 present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.physical, surface, &present_mode_count, nullptr);
    assert(present_mode_count < MAX_PRESENT_MODE_COUNT);
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

    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
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

    // required to safely destroy after creating new swapchain.
    if (create_info.oldSwapchain != nullptr) vkDestroySwapchainKHR(gpu.logical, create_info.oldSwapchain, nullptr);

    // retrieve images
    VkImage placeholder[MAX_IMAGES];
    vkGetSwapchainImagesKHR(gpu.logical, swapchain.handle, &swapchain.num_images, nullptr);
    assert(swapchain.num_images <= MAX_IMAGES);
    vkGetSwapchainImagesKHR(gpu.logical, swapchain.handle, &swapchain.num_images, placeholder);
}

void free_swapchain(Swapchain& swapchain) {
    if (swapchain.handle) {
        vkDestroySwapchainKHR(gpu.logical, swapchain.handle, nullptr);
    }
    if (swapchain.surface) {
        vkDestroySurfaceKHR(instance, swapchain.surface, nullptr);
    }
}
