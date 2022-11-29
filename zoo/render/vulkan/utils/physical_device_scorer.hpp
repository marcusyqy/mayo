#pragma once

#include <vulkan/vulkan.h>

namespace zoo::render {

bool check_device_features_met(VkPhysicalDeviceFeatures features) noexcept {
    return true;
}

bool check_device_properties_met(
    VkPhysicalDeviceProperties properties) noexcept {
    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

struct physical_device_scorer {
    physical_device_scorer(VkPhysicalDevice device) noexcept {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(device, &device_features);

        device_features_met_ = check_device_features_met(device_features);
        device_properties_met_ = check_device_properties_met(device_properties);
    }

    operator bool() const noexcept {
        return device_features_met_ && device_properties_met_;
    }

    bool device_features_met_ = false;
    bool device_properties_met_ = false;
};
} // namespace zoo::render
