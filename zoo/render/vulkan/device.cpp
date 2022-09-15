
#include "Device.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

Device::Device(VkInstance instance, VkPhysicalDevice physical_device) noexcept
    : physical_(physical_device) {}

auto Device::reset() noexcept -> void {
    if (logical_ != VK_NULL_HANDLE) {
        vkDestroyDevice(logical_, nullptr);
        logical_ = VK_NULL_HANDLE;
    }
}

Device::~Device() noexcept { reset(); }

/*
    release device resources
*/
auto Device::release_device_resource(VkFence fence) noexcept -> void {
    if (fence != VK_NULL_HANDLE)
        vkDestroyFence(logical_, fence, nullptr);
}

} // namespace zoo::render::vulkan
