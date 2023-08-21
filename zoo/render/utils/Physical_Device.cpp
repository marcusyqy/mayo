#include "Physical_Device.hpp"
#include <algorithm>
#include <iterator>

// #define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3native.h>

namespace zoo::render::utils {

Physical_Device::Physical_Device(underlying_type underlying) noexcept : underlying_{ underlying } {
    query_properties_and_features();
}

void Physical_Device::query_properties_and_features() noexcept {
    vkGetPhysicalDeviceProperties(underlying_, &properties_);

    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_feature = {};
    shader_draw_parameters_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shader_draw_parameters_feature.pNext = nullptr;
    shader_draw_parameters_feature.shaderDrawParameters = VK_TRUE;

    VkPhysicalDeviceFeatures2 features;
    features.pNext = &shader_draw_parameters_feature;
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    vkGetPhysicalDeviceFeatures2(underlying_, &features);

    shader_draw_parameters_enabled_ = shader_draw_parameters_feature.shaderDrawParameters == VK_TRUE;
    features_                       = features.features;
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(underlying_, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(underlying_, &queue_family_count, queue_families.data());
        uint32_t idx{};
        std::transform(
            std::begin(queue_families),
            std::end(queue_families),
            std::back_inserter(queue_family_properties_),
            [&idx](const VkQueueFamilyProperties& props) -> Queue_Family_Properties {
                return { idx++, props };
            });
    }
    {
        uint32_t extension_count{};
        vkEnumerateDeviceExtensionProperties(underlying_, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(extension_count);

        vkEnumerateDeviceExtensionProperties(underlying_, nullptr, &extension_count, available_extensions.data());

        std::transform(
            std::begin(available_extensions),
            std::end(available_extensions),
            std::inserter(device_extensions_, std::end(device_extensions_)),
            [](const VkExtensionProperties& props) -> std::string { return props.extensionName; });
    }
}

bool Physical_Device::is_discrete() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}
bool Physical_Device::is_integrated() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}
bool Physical_Device::is_virtual() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
}
bool Physical_Device::is_cpu() const noexcept { return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU; }
bool Physical_Device::is_other() const noexcept { return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER; }

const VkPhysicalDeviceFeatures& Physical_Device::features() const noexcept { return features_; }

std::string_view Physical_Device::name() const noexcept { return properties_.deviceName; }

Physical_Device::id_type Physical_Device::id() const noexcept { return properties_.deviceID; }

VkPhysicalDeviceLimits Physical_Device::limits() const noexcept { return properties_.limits; }

bool Physical_Device::has_geometry_shader() const noexcept { return features_.geometryShader; }

bool Physical_Device::has_present(const Queue_Family_Properties& family_props, VkInstance instance) const noexcept {
    // glfwGetPhysicalDevicePresentationSupport is merely an abstraction of the
    // corresponding platform-specific functions
    //
    // - vkGetPhysicalDeviceWin32PresentationSupportKHR,
    // - vkGetPhysicalDeviceXlibPresentationSupportKHR,
    // - vkGetPhysicalDeviceWaylandPresentationSupportKHR and
    // - vkGetPhysicalDeviceMirPresentationSupportKHR and is meant for use
    // before surface creation.
    //
    return glfwGetPhysicalDevicePresentationSupport(instance, underlying_, family_props.index()) == GLFW_TRUE;
}

bool Physical_Device::has_required_extension(std::string_view extension_name) const noexcept {
    return device_extensions_.count(extension_name.data()) > 0;
}
} // namespace zoo::render::utils
