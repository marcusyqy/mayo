
#include "device.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

device::device([[maybe_unused]] VkInstance instance,
    VkPhysicalDevice physical_device) noexcept
    : physical_(physical_device) {}

void device::reset() noexcept {
    if (logical_ != VK_NULL_HANDLE) {
        vkDestroyDevice(logical_, nullptr);
        logical_ = VK_NULL_HANDLE;
    }
}

device::~device() noexcept { reset(); }

/*
    release device resources
*/
void device::release_device_resource(VkFence fence) noexcept {
    if (fence != VK_NULL_HANDLE)
        vkDestroyFence(logical_, fence, nullptr);
}

} // namespace zoo::render::vulkan
