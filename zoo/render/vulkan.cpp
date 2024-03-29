#include "vulkan.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace zoo::vk {

namespace {

enum : u32 { MAX_FORMAT_COUNT = 100, MAX_PRESENT_MODE_COUNT = 100 };

// @TODO: don't know if this is fine.
std::vector<const char*> get_layers() noexcept {

    if (ENABLE_VALIDATION) {
        const char* validation_layer{ "VK_LAYER_KHRONOS_validation" };
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(static_cast<size_t>(layer_count));
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const auto& available_layer : available_layers) {
            if (strcmp(validation_layer, available_layer.layerName)) {
                return std::vector<const char*>{ validation_layer };
            }
        }
    }

    return std::vector<const char*>();
}

std::vector<const char*> get_extensions() noexcept {
    std::vector<const char*> extensions{};
    uint32_t glfw_extensions_count;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    std::copy(glfw_extensions, glfw_extensions + glfw_extensions_count, std::back_inserter(extensions));

    if (ENABLE_VALIDATION) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

const char* get_scope_string(VkSystemAllocationScope scope) {
    switch (scope) {
        case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: return "COMMAND";
        case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: return "OBJECT";
        case VK_SYSTEM_ALLOCATION_SCOPE_CACHE: return "CACHE";
        case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: return "DEVICE";
        case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: return "INSTANCE";
        default: return "UNKNOWN";
    }
}

// @TODO: Implement CPU allocator. ALLOCATION CALLBACKS
void* allocation_fn(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
    const char* scope = get_scope_string(allocation_scope);
    ZOO_LOG_INFO("Logging allocation function[{}]size : {}, alignment : {}", scope, size, alignment);
    return malloc(size);
}

void* reallocation_fn(
    void* user_data,
    void* original,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope allocation_scope) {
    const char* scope = get_scope_string(allocation_scope);
    ZOO_LOG_INFO("Logging reallocation function[{}]size : {}, alignment : {}", scope, size, alignment);
    return realloc(original, size);
}

void free_fn(void* user_data, void* memory) { return free(memory); }

void internal_allocation_notification_fn(
    void* user_data,
    size_t size,
    VkInternalAllocationType allocation_type,
    VkSystemAllocationScope allocation_scope) {
    const char* scope = get_scope_string(allocation_scope);
    ZOO_LOG_INFO("Logging internal_allocation_notification_fn [{}]size : {}, alignment", scope, size);
}

void internal_free_notification_fn(
    void* user_data,
    size_t size,
    VkInternalAllocationType allocation_type,
    VkSystemAllocationScope allocation_scope) {
    const char* scope = get_scope_string(allocation_scope);
    ZOO_LOG_INFO("Logging free_notification function[{}] : size : {}", scope, size);
}

VkInstance create_instance(const VkAllocationCallbacks& allocation_callbacks) noexcept {
    VkApplicationInfo app_info{ .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                .pNext              = nullptr, // for now
                                .pApplicationName   = "Zoo Engine",
                                .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
                                .pEngineName        = "Zoo Engine",
                                .engineVersion      = VK_MAKE_VERSION(0, 0, 0),
                                .apiVersion         = API_VERSION };

    const auto& enabled_layers     = get_layers();
    const auto& enabled_extensions = get_extensions();

    VkInstanceCreateInfo create_info{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = static_cast<u32>(enabled_layers.size()),
        .ppEnabledLayerNames     = enabled_layers.data(),
        .enabledExtensionCount   = static_cast<u32>(enabled_extensions.size()),
        .ppEnabledExtensionNames = enabled_extensions.data(),
    };

    VkInstance instance = nullptr;
    VK_EXPECT_SUCCESS(vkCreateInstance(&create_info, &allocation_callbacks, &instance));
    return instance;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    [[maybe_unused]] void* user_data) noexcept {

    const char* prepend = nullptr;
    if (type >= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) prepend = "PERFORMANCE";
    else if (type >= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        prepend = "VALIDATION";
    else if (type >= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        prepend = "GENERAL";

    static constexpr char validation_message[] = "<{}> :|vulkan|: {}";
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        ZOO_LOG_ERROR(validation_message, prepend, callback_data->pMessage);
    else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        ZOO_LOG_WARN(validation_message, prepend, callback_data->pMessage);
    else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        ZOO_LOG_INFO(validation_message, prepend, callback_data->pMessage);
    else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        ZOO_LOG_TRACE(validation_message, prepend, callback_data->pMessage);

    return VK_FALSE;
}

VkDebugUtilsMessengerEXT
    create_debug_utils_messenger_ext(VkInstance instance, const VkAllocationCallbacks& allocator) noexcept {

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
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

    VkDebugUtilsMessengerEXT debug_messenger;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (!func) {
        ZOO_LOG_INFO("Debug utils messenger failed with {}", string_VkResult(VK_ERROR_EXTENSION_NOT_PRESENT));
        return VK_NULL_HANDLE;
    }

    VkResult result = func(instance, &create_info, &allocator, &debug_messenger);

    if (result != VK_SUCCESS) {
        ZOO_LOG_INFO("Debug utils messenger failed with {}", string_VkResult(result));
        return VK_NULL_HANDLE;
    }

    return debug_messenger;
}

void destroy_debug_utils_messenger_ext(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks& allocator) noexcept {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, &allocator);
    }
}

struct Device_Selection {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    u32 graphics_queue               = {};
    u32 present_queue                = {};
};

Device_Selection select_device(VkInstance instance, VkSurfaceKHR surface) noexcept {
    constexpr auto MAX_NUMBER_DEVICES = 10;
    VkPhysicalDevice devices[MAX_NUMBER_DEVICES];
    u32 device_count = 0;
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
    if (device_count == 0) {
        ZOO_LOG_ERROR("Devices cannot be 0 for vkEnumeratePhysicalDevices!");

        // @TODO: this needs to fail (assertion) or some sort of termination that
        // tells us something meaningful which users can report to us about.
        return { VK_NULL_HANDLE };
    }

    ZOO_ASSERT(device_count < MAX_NUMBER_DEVICES);
    VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &device_count, devices));

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

        u32 extension_count{};
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

        constexpr u32 MAX_EXTENSIONS = 300;
        ZOO_ASSERT(extension_count < MAX_EXTENSIONS);
        VkExtensionProperties available_extensions[MAX_EXTENSIONS];
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions);

        u32 j = 0;
        for (; j < extension_count; ++j) {
            auto& props = available_extensions[j];
            if (strcmp(props.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) break;
        }

        // did not find.
        if (j == extension_count) continue;

        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

        constexpr u32 MAX_QUEUE_COUNT = 20;

        VkQueueFamilyProperties queue_families[MAX_QUEUE_COUNT];
        ZOO_ASSERT(queue_family_count < MAX_QUEUE_COUNT);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
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

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            ZOO_LOG_INFO("Chose {}.", properties.deviceName);
            return { physical_device, graphics_queue_idx, present_queue_idx };
        }

        most_compatible                = i;
        most_compatible_graphics_queue = graphics_queue_idx;
        most_compatible_present_queue  = present_queue_idx;
    }

    ZOO_LOG_WARN("Defaulting to most compatible gpu that is not discrete gpu");
    return { devices[most_compatible], most_compatible_graphics_queue, most_compatible_present_queue };
}

VkDevice create_device(
    VkInstance instance,
    VkPhysicalDevice physical_device,
    u32 graphics_queue_family_idx,
    u32 present_queue_family_idx,
    const VkAllocationCallbacks& allocation_callbacks) {

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    constexpr u32 MAX_QUEUE_COUNT = 20;
    VkQueueFamilyProperties queue_families[MAX_QUEUE_COUNT];
    ZOO_ASSERT(queue_family_count < MAX_QUEUE_COUNT);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info[2] = {};

    queue_create_info[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[0].queueFamilyIndex = graphics_queue_family_idx;
    queue_create_info[0].pQueuePriorities = &queue_priority;
    queue_create_info[0].queueCount       = 1;

    queue_create_info[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[1].queueFamilyIndex = present_queue_family_idx;
    queue_create_info[1].pQueuePriorities = &queue_priority;
    queue_create_info[1].queueCount       = 1;

    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_feature = {};
    shader_draw_parameters_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shader_draw_parameters_feature.pNext = nullptr;
    shader_draw_parameters_feature.shaderDrawParameters = VK_TRUE;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    const char* device_extension{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo create_info{};
    create_info.pNext                   = &shader_draw_parameters_feature;
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount    = present_queue_family_idx == graphics_queue_family_idx ? 1 : 2;
    create_info.pQueueCreateInfos       = queue_create_info;
    create_info.pEnabledFeatures        = &features;
    create_info.ppEnabledExtensionNames = &device_extension;
    create_info.enabledExtensionCount   = 1;

    const char* validation_layer = "VK_LAYER_KHRONOS_validation";
    if (ENABLE_VALIDATION) {
        create_info.enabledLayerCount   = 1;
        create_info.ppEnabledLayerNames = std::addressof(validation_layer);
    } else {
        create_info.enabledLayerCount   = 0;
        create_info.ppEnabledLayerNames = nullptr;
    }

    VkDevice device;
    VK_EXPECT_SUCCESS(vkCreateDevice(physical_device, &create_info, &allocation_callbacks, &device));
    return device;
}

VkCommandPool create_command_pool(
    VkDevice device,
    u32 index,
    const VkAllocationCallbacks& allocation_callbacks,
    VkCommandPoolCreateFlags create_flags) {
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.flags            = create_flags;
    pool_create_info.queueFamilyIndex = index;

    VkCommandPool command_pool;
    VK_EXPECT_SUCCESS(vkCreateCommandPool(device, &pool_create_info, &allocation_callbacks, &command_pool));
    return command_pool;
}

VkSurfaceFormatKHR choose_surface_format(const VkSurfaceFormatKHR* available_formats, u32 count) noexcept {
    // Select Surface Format (from ImGui):
    // const VkFormat requestSurfaceImageFormat[]     = { VK_FORMAT_B8G8R8A8_UNORM,
    //                                                    VK_FORMAT_R8G8B8A8_UNORM,
    //                                                    VK_FORMAT_B8G8R8_UNORM,
    //                                                    VK_FORMAT_R8G8B8_UNORM };
    for (u32 i = 0; i < count; ++i) {
        const auto& format = available_formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return available_formats[0];
}

VkPresentModeKHR choose_present_mode(const VkPresentModeKHR* available_presentation_modes, u32 count) noexcept {
    for (u32 i = 0; i < count; ++i) {
        const auto& mode = available_presentation_modes[i];
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

void create_or_recreate_swapchain(
    Window_Data& window,
    VkDevice device,
    VkPhysicalDevice physical_device,
    s32 width,
    s32 height,
    Image_Registry& image_registry,
    const VkAllocationCallbacks& allocation_callbacks) {

    VkSurfaceCapabilitiesKHR capabilities = {};
    VkSurfaceFormatKHR formats[MAX_FORMAT_COUNT];

    VkPresentModeKHR present_modes[MAX_PRESENT_MODE_COUNT];

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, window.surface, &capabilities);

    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, window.surface, &format_count, nullptr);
    if (format_count != 0) {
        ZOO_ASSERT(format_count < MAX_FORMAT_COUNT);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, window.surface, &format_count, formats);
    }

    u32 present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, window.surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        ZOO_ASSERT(present_mode_count < MAX_PRESENT_MODE_COUNT);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, window.surface, &present_mode_count, present_modes);
    }

    //
    window.surface_format    = choose_surface_format(formats, format_count);
    auto chosen_present_mode = choose_present_mode(present_modes, present_mode_count);
    auto chosen_extent       = choose_extent(capabilities, width, height);

    window.width  = chosen_extent.width;
    window.height = chosen_extent.height;

    u32 image_count =
        std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR create_info{ .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                          .pNext                 = nullptr,
                                          .flags                 = 0,
                                          .surface               = window.surface,
                                          .minImageCount         = image_count,
                                          .imageFormat           = window.surface_format.format,
                                          .imageColorSpace       = window.surface_format.colorSpace,
                                          .imageExtent           = chosen_extent,
                                          .imageArrayLayers      = 1,
                                          .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                          .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
                                          .queueFamilyIndexCount = 0,
                                          .pQueueFamilyIndices   = nullptr,
                                          .preTransform          = capabilities.currentTransform,
                                          .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                          .presentMode           = chosen_present_mode,
                                          .clipped               = VK_TRUE,
                                          .oldSwapchain          = window.swapchain };

    VK_EXPECT_SUCCESS(vkCreateSwapchainKHR(device, &create_info, &allocation_callbacks, &window.swapchain));

    // required to safely destroy after creating new swapchain.
    if (create_info.oldSwapchain != nullptr) {
        vkDestroySwapchainKHR(device, create_info.oldSwapchain, &allocation_callbacks);
        for (u32 i = 0; i < window.num_images; ++i) {
            // must not have any allocation.
            ZOO_ASSERT(image_registry.pool[window.images[i]].image.mem == VK_NULL_HANDLE);
            image_registry.pool.erase(window.images[i]);
        }
    }

    // retrieve images
    vkGetSwapchainImagesKHR(device, window.swapchain, &window.num_images, nullptr);
    VkImage placeholder[MAX_IMAGES];
    vkGetSwapchainImagesKHR(device, window.swapchain, &window.num_images, placeholder);

    for (u32 i = 0; i < window.num_images; ++i) {
        auto index        = image_registry.pool.create();
        auto& data        = image_registry.pool[index];
        data.image.vk     = placeholder[i];
        data.image.handle = index;
        data.image.mem    = VK_NULL_HANDLE;
        window.images[i]  = index;
    }
    //@BOOKMARK
}

VmaAllocator create_vma_allocator(VkInstance instance, VkDevice device, VkPhysicalDevice physical_device) {
    VmaAllocatorCreateInfo create_info{};
    create_info.vulkanApiVersion = API_VERSION;
    create_info.physicalDevice   = physical_device;
    create_info.device           = device;
    create_info.instance         = instance;
    VmaAllocator allocator       = VK_NULL_HANDLE;
    VK_EXPECT_SUCCESS(vmaCreateAllocator(&create_info, &allocator));
    return allocator;
}

void create_image_registry([[maybe_unused]] Image_Registry& registry) {
    // ... does nothing lol.
    // supposedly gets the vma pool but let's delegate that for later.
}

} // namespace

Render_Context allocate_render_context(Window& window) noexcept {
    Render_Context render_context;
    render_context.allocation_callbacks = { .pUserData             = nullptr,
                                            .pfnAllocation         = allocation_fn,
                                            .pfnReallocation       = reallocation_fn,
                                            .pfnFree               = free_fn,
                                            .pfnInternalAllocation = internal_allocation_notification_fn,
                                            .pfnInternalFree       = internal_free_notification_fn };

    render_context.instance = create_instance(render_context.allocation_callbacks);
    render_context.debug =
        create_debug_utils_messenger_ext(render_context.instance, render_context.allocation_callbacks);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VK_EXPECT_SUCCESS(glfwCreateWindowSurface(
        render_context.instance,
        window.impl(),
        &render_context.allocation_callbacks,
        &surface));

    auto& window_data   = render_context.windows[render_context.num_windows++];
    window_data.context = window.impl();
    window_data.surface = surface;

    auto [physical_device, graphics_queue_family_idx, present_queue_family_idx] =
        select_device(render_context.instance, window_data.surface);
    render_context.physical_device = physical_device;

    render_context.device = create_device(
        render_context.instance,
        render_context.physical_device,
        graphics_queue_family_idx,
        present_queue_family_idx,
        render_context.allocation_callbacks);

    render_context.allocator =
        create_vma_allocator(render_context.instance, render_context.device, render_context.physical_device);

    vkGetDeviceQueue(render_context.device, graphics_queue_family_idx, 0, &render_context.graphics_queue);
    render_context.graphics_command_pool = create_command_pool(
        render_context.device,
        graphics_queue_family_idx,
        render_context.allocation_callbacks,
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    if (graphics_queue_family_idx != present_queue_family_idx) {
        vkGetDeviceQueue(render_context.device, present_queue_family_idx, 0, &render_context.present_queue);
        render_context.present_command_pool = create_command_pool(
            render_context.device,
            present_queue_family_idx,
            render_context.allocation_callbacks,
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    } else {
        render_context.present_queue        = render_context.graphics_queue;
        render_context.present_command_pool = render_context.graphics_command_pool;
    }

    create_image_registry(render_context.image_registry);

    create_or_recreate_swapchain(
        window_data,
        render_context.device,
        render_context.physical_device,
        window.width(),
        window.height(),
        render_context.image_registry,
        render_context.allocation_callbacks);

    return render_context;
}

void free_render_context(Render_Context& render_context) noexcept {
    vkDestroyCommandPool(
        render_context.device,
        render_context.graphics_command_pool,
        &render_context.allocation_callbacks);

    if (render_context.present_command_pool != render_context.graphics_command_pool) {
        vkDestroyCommandPool(
            render_context.device,
            render_context.present_command_pool,
            &render_context.allocation_callbacks);
    }

    vmaDestroyAllocator(render_context.allocator);
    vkDestroyDevice(render_context.device, &render_context.allocation_callbacks);

    for (u32 i = 0; i < render_context.num_windows; ++i) {
        vkDestroySurfaceKHR(
            render_context.instance,
            render_context.windows[i].surface,
            &render_context.allocation_callbacks);
    }

    destroy_debug_utils_messenger_ext(
        render_context.instance,
        render_context.debug,
        render_context.allocation_callbacks);
    vkDestroyInstance(render_context.instance, &render_context.allocation_callbacks);

    memset(&render_context, 0, sizeof(Render_Context));
}

} // namespace zoo::vk
