
#include "Device.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

Device::Device(VkInstance instance) noexcept {}
void Device::cleanup() noexcept { vkDestroyDevice(logical_, nullptr); }

/*
    release device resources
*/
void Device::release_device_resource(VkFence fence) {
    if (fence != VK_NULL_HANDLE)
        vkDestroyFence(logical_, fence, nullptr);
}

} // namespace zoo::render::vulkan