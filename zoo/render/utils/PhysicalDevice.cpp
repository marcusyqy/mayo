#include "PhysicalDevice.hpp"
#include <algorithm>
#include <iterator>

// #define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3native.h>

namespace zoo::render::utils {

PhysicalDevice::PhysicalDevice(underlying_type underlying) noexcept
    : underlying_{underlying} {
    query_properties_and_features();
}

void PhysicalDevice::query_properties_and_features() noexcept {
    vkGetPhysicalDeviceProperties(underlying_, &properties_);
    vkGetPhysicalDeviceFeatures(underlying_, &features_);
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            underlying_, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(
            underlying_, &queue_family_count, queue_families.data());
        uint32_t idx{};
        std::transform(std::begin(queue_families), std::end(queue_families),
            std::back_inserter(queue_family_properties_),
            [&idx](
                const VkQueueFamilyProperties& props) -> QueueFamilyProperties {
                return {idx++, props};
            });
    }
    {
        uint32_t extension_count{};
        vkEnumerateDeviceExtensionProperties(
            underlying_, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(
            extension_count);

        vkEnumerateDeviceExtensionProperties(underlying_, nullptr,
            &extension_count, available_extensions.data());

        std::transform(std::begin(available_extensions),
            std::end(available_extensions),
            std::inserter(device_extensions_, std::end(device_extensions_)),
            [](const VkExtensionProperties& props) -> std::string_view {
                return props.extensionName;
            });
    }
}

bool PhysicalDevice::is_discrete() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}
bool PhysicalDevice::is_integrated() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}
bool PhysicalDevice::is_virtual() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
}
bool PhysicalDevice::is_cpu() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
}
bool PhysicalDevice::is_other() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER;
}

const VkPhysicalDeviceFeatures& PhysicalDevice::features() const noexcept {
    return features_;
}

std::string_view PhysicalDevice::name() const noexcept {
    return properties_.deviceName;
}

PhysicalDevice::id_type PhysicalDevice::id() const noexcept {
    return properties_.deviceID;
}

VkPhysicalDeviceLimits PhysicalDevice::limits() const noexcept {
    return properties_.limits;
}

bool PhysicalDevice::has_geometry_shader() const noexcept {
    return features_.geometryShader;
}

bool PhysicalDevice::has_present(const QueueFamilyProperties& family_props,
    VkInstance instance) const noexcept {
    // glfwGetPhysicalDevicePresentationSupport is merely an abstraction of the
    // corresponding platform-specific functions
    //
    // - vkGetPhysicalDeviceWin32PresentationSupportKHR,
    // - vkGetPhysicalDeviceXlibPresentationSupportKHR,
    // - vkGetPhysicalDeviceWaylandPresentationSupportKHR and
    // - vkGetPhysicalDeviceMirPresentationSupportKHR and is meant for use
    // before surface creation.
    //
    return glfwGetPhysicalDevicePresentationSupport(
               instance, underlying_, family_props.index()) == GLFW_TRUE;
}

bool PhysicalDevice::has_required_extension(
    std::string_view extension_name) const noexcept {
    return device_extensions_.count(extension_name) > 0;
}
} // namespace zoo::render::utils
