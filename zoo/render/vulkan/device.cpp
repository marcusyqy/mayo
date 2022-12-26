#include "device.hpp"
#include "render/fwd.hpp"

namespace zoo::render::vulkan {

// TODO: since we need to create the queues at the start should we also just
// initialize the engine with settings that will determine the queues that
// is important to us?
device::device([[maybe_unused]] VkInstance instance,
    utils::physical_device pdevice,
    const utils::queue_family_properties& family_props,
    const platform::render::query& query) noexcept
    : physical_(pdevice) {

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family_props.index();

    // TODO: check if we can just have size
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
    create_info.enabledExtensionCount = 0;

    // this has been deprecated on newer versions of VULKAN
    //
    // TODO: remove this when proven not to be needed. (only need to be set in
    // VkInstance which is `engine.cpp`)
    //
    const char* validation_layer{"VK_LAYER_KHRONOS_validation"};
    if (query.get_params().validation_) {
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = &validation_layer;
    } else {
        create_info.enabledLayerCount = 0;
    }

    VK_EXPECT_SUCCESS(
        vkCreateDevice(physical_, &create_info, nullptr, &logical_),
        [this]([[maybe_unused]] VkResult result) {
            // not sure if device will be set to nullptr after the end.
            // need a `then` callback.
            logical_ = nullptr;
        });

    // TODO: create a queue. where should this be stored?
    VkQueue queue;
    vkGetDeviceQueue(logical_, family_props.index(), 0, &queue);
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
