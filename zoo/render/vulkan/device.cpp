#include "device.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

physical_device::physical_device(underlying_type underlying) noexcept
    : underlying_{underlying} {}

void physical_device::query_properties_and_features() noexcept {
    vkGetPhysicalDeviceProperties(underlying_, &properties_);
    vkGetPhysicalDeviceFeatures(underlying_, &features_);
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

device::device(
    [[maybe_unused]] VkInstance instance, physical_device pdevice) noexcept
    : physical_(pdevice) {
    // create logical device here.
}

void device::reset() noexcept {
    if (logical_ != nullptr) {
        vkDestroyDevice(logical_, nullptr);
        logical_ = nullptr;
    }
}

device::~device() noexcept { reset(); }

/*
    release device resources for each vulkan resource
*/
void device::release_device_resource(VkFence fence) noexcept {
    if (fence != nullptr)
        vkDestroyFence(logical_, fence, nullptr);
}

} // namespace zoo::render::vulkan
