#include "messenger.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace zoo::render::vulkan::debug {

namespace {

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
        ZOO_LOG_ERROR(validation_message, prepend, callback_data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        ZOO_LOG_WARN(validation_message, prepend, callback_data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        ZOO_LOG_INFO(validation_message, prepend, callback_data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        ZOO_LOG_TRACE(validation_message, prepend, callback_data->pMessage);
    }
    return VK_FALSE;
}

VkResult create_debug_utils_messenger_ext(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger) noexcept {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger_ext(VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator) noexcept {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, allocator);
    }
}
} // namespace

messenger::messenger(VkInstance instance) noexcept
    : instance_(instance), debug_messenger_(VK_NULL_HANDLE) {

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    {
        create_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
        create_info.pUserData = nullptr; // Optional
    }

    VkResult result = create_debug_utils_messenger_ext(
        instance_, &create_info, nullptr, &debug_messenger_);

    if (result != VK_SUCCESS) {
        ZOO_LOG_INFO(
            "Debug utils messenger failed with {}", string_VkResult(result));
        debug_messenger_ = VK_NULL_HANDLE; // set back to null just to be sure.
    }
}

messenger::~messenger() noexcept { reset(); }

void messenger::reset() noexcept {
    if (debug_messenger_ != VK_NULL_HANDLE) {
        destroy_debug_utils_messenger_ext(instance_, debug_messenger_, nullptr);
        debug_messenger_ = VK_NULL_HANDLE;
    }
}

messenger::messenger(messenger&& other) noexcept
    : instance_(VK_NULL_HANDLE), debug_messenger_(VK_NULL_HANDLE) {
    *this = std::move(other);
}

messenger& messenger::operator=(messenger&& other) noexcept {
    std::swap(instance_, other.instance_);
    std::swap(debug_messenger_, other.debug_messenger_);
    return *this;
}

} // namespace zoo::render::vulkan::debug
