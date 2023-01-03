#include "physical_device.hpp"
#include <algorithm>
#include <iterator>

// #define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3native.h>

namespace zoo::render::utils {

physical_device::physical_device(underlying_type underlying) noexcept :
    underlying_{underlying} {
    query_properties_and_features();
}

void physical_device::query_properties_and_features() noexcept {
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
            [&idx](const VkQueueFamilyProperties& props)
                -> queue_family_properties {
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

bool physical_device::is_discrete() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}
bool physical_device::is_integrated() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}
bool physical_device::is_virtual() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
}
bool physical_device::is_cpu() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
}
bool physical_device::is_other() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER;
}

const VkPhysicalDeviceFeatures& physical_device::features() const noexcept {
    return features_;
}

std::string_view physical_device::name() const noexcept {
    return properties_.deviceName;
}

physical_device::id_type physical_device::id() const noexcept {
    return properties_.deviceID;
}

VkPhysicalDeviceLimits physical_device::limits() const noexcept {
    return properties_.limits;
}

bool physical_device::has_geometry_shader() const noexcept {
    return features_.geometryShader;
}

bool physical_device::has_present(const queue_family_properties& family_props,
    VkInstance instance) const noexcept {
    return glfwGetPhysicalDevicePresentationSupport(
               instance, underlying_, family_props.index()) == GLFW_TRUE;
}

bool physical_device::has_required_extension(
    std::string_view extension_name) const noexcept {
    return device_extensions_.count(extension_name) > 0;
}
} // namespace zoo::render::utils
