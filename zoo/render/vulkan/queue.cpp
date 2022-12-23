
#include "queue.hpp"

namespace zoo::render::vulkan {

queue::queue(utils::queue_family_properties family_props) noexcept {
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family_props.index();
    // TODO: check if wee can just have 1
    queue_create_info.queueCount = 1; // family_props.size();

    // TODO: check priority out
    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    // TODO: actually finish writing this
}
} // namespace zoo::render::vulkan
