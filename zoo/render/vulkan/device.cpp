
#include "device.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

device::device([[maybe_unused]] VkInstance instance,
    VkPhysicalDevice physical_device) noexcept
    : physical_(physical_device) {}

auto device::reset() noexcept -> void {
    if (logical_ != VK_NULL_HANDLE) {
        vkDestroyDevice(logical_, nullptr);
        logical_ = VK_NULL_HANDLE;
    }
}

device::~device() noexcept { reset(); }

/*
    release device resources
*/
auto device::release_device_resource(VkFence fence) noexcept -> void {
    if (fence != VK_NULL_HANDLE)
        vkDestroyFence(logical_, fence, nullptr);
}

} // namespace zoo::render::vulkan
