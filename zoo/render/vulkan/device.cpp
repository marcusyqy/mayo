#include "device.hpp"
#include "render/fwd.hpp"

namespace zoo::render::vulkan {

device::device([[maybe_unused]] VkInstance instance,
    utils::physical_device pdevice,
    const utils::queue_family_properties& family_props) noexcept
    : physical_(pdevice) {
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family_props.index();
    // TODO: check if wee can just have 1
    queue_create_info.queueCount = 1; // family_props.size();

    // TODO: check priority out
    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    // create logical device here.
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.pEnabledFeatures = &(physical_.features());

    VK_EXPECT_SUCCESS(
        vkCreateDevice(physical_, &create_info, nullptr, &logical_));
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
