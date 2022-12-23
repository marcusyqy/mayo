#include "device.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

device::device([[maybe_unused]] VkInstance instance,
    utils::physical_device pdevice) noexcept
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
