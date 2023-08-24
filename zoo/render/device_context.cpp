#include "device_context.hpp"
#include "core/fwd.hpp"
#include "render/fwd.hpp"
#include "scene/command_buffer.hpp"

namespace zoo::render {
namespace {
const char* device_extension{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

} // namespace

// TODO: since we need to create the queues at the start should we also just
// initialize the engine with settings that will determine the queues that
// is important to us?
Device_Context::Device_Context(
    [[maybe_unused]] VkInstance instance,
    utils::Physical_Device pdevice,
    const utils::Queue_Family_Properties& family_props,
    const render::Query& query) noexcept :
    physical_(pdevice),
    queue_properties_{ family_props } {

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_properties_.index();

    // TODO: check if we can just have size
    queue_create_info.queueCount = 1;

    // TODO: check priority out
    float queue_priority               = 1.0f;
    queue_create_info.pQueuePriorities = std::addressof(queue_priority);

    /**
     * If the VkPhysicalDeviceShaderDrawParametersFeatures structure is included in the pNext chain of the
     * VkPhysicalDeviceFeatures2 structure passed to vkGetPhysicalDeviceFeatures2, it is filled in to indicate whether
     * each corresponding feature is supported. VkPhysicalDeviceShaderDrawParametersFeatures can also be used in the
     * pNext chain of VkDeviceCreateInfo to selectively enable these features.
     */
    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_feature = {};
    shader_draw_parameters_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shader_draw_parameters_feature.pNext = nullptr;
    shader_draw_parameters_feature.shaderDrawParameters = VK_TRUE;

    // create logical device here.
    VkDeviceCreateInfo create_info{};
    create_info.pNext                = &shader_draw_parameters_feature;
    create_info.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos    = std::addressof(queue_create_info);
    create_info.pEnabledFeatures     = std::addressof(physical_.features());

    create_info.ppEnabledExtensionNames = &device_extension;
    create_info.enabledExtensionCount   = 1;

    // this has been deprecated on newer versions of VULKAN
    //
    // TODO: remove this when proven not to be needed. (only need to be set in
    // VkInstance which is `engine.cpp`)
    const char* validation_layer = "VK_LAYER_KHRONOS_validation";
    if (query.get_params().validation) {
        create_info.enabledLayerCount   = 1;
        create_info.ppEnabledLayerNames = std::addressof(validation_layer);
    } else {
        create_info.enabledLayerCount = 0;
    }

    VK_EXPECT_SUCCESS(
        vkCreateDevice(physical_, &create_info, nullptr, &logical_),
        [this]([[maybe_unused]] VkResult result) {
            // not sure if device will be set to nullptr after the
            // end. need a `then` callback.
            logical_ = nullptr;
            std::abort(); // maybe we don't need this since it's
                          // already going to destroy itself.
        });

    // create a queue. where should this be stored?
    // TODO: need to consider that will also have two different types of indices
    // and two different types of queues for present and graphics.
    vkGetDeviceQueue(logical_, queue_properties_.index(), 0, &queue_);

    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_create_info.queueFamilyIndex = queue_properties_.index();
    VK_EXPECT_SUCCESS(vkCreateCommandPool(logical_, &pool_create_info, nullptr, &command_pool_));

    allocator_.emplace(instance, logical_, physical_);
}

void Device_Context::reset() noexcept {
    if (logical_ != nullptr) {
        wait();
        allocator_.reset();
        if (command_pool_ != nullptr) vkDestroyCommandPool(logical_, command_pool_, nullptr);

        vkDestroyDevice(logical_, nullptr);
        logical_ = nullptr;
    }
}

Device_Context::~Device_Context() noexcept { reset(); }

VkCommandBuffer Device_Context::vk_command_buffer_from_pool(Operation op) const noexcept {
    static_cast<void>(op);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool_;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer{};
    VK_EXPECT_SUCCESS(vkAllocateCommandBuffers(logical_, &alloc_info, &command_buffer));

    return command_buffer;
}

/*
    release device resources for each vulkan resource
*/
void Device_Context::release_device_resource(VkFence fence) noexcept {
    if (fence != nullptr) vkDestroyFence(logical_, fence, nullptr);
}

void Device_Context::release_device_resource(VkRenderPass renderpass) noexcept {
    if (renderpass != nullptr) vkDestroyRenderPass(logical_, renderpass, nullptr);
}

void Device_Context::release_device_resource(VkSemaphore semaphore) noexcept {
    if (semaphore != nullptr) vkDestroySemaphore(logical_, semaphore, nullptr);
}

void Device_Context::release_device_resource(VkBuffer buffer) noexcept {
    if (buffer != nullptr) vkDestroyBuffer(logical_, buffer, nullptr);
}

void Device_Context::release_device_resource(VkDeviceMemory device_memory) noexcept {
    if (device_memory != nullptr) vkFreeMemory(logical_, device_memory, nullptr);
}

VkQueue Device_Context::retrieve(Operation op) const noexcept {

    switch (op) {
        case Operation::graphics: [[fallthrough]];
        case Operation::transfer: [[fallthrough]];
        case Operation::present: return queue_;
        default: ZOO_ASSERT(false, "not supporting other queue types yet."); break;
    }
    return queue_;
}

void Device_Context::wait() noexcept {
    if (logical_ != nullptr) vkDeviceWaitIdle(logical_);
    else
        ZOO_LOG_ERROR("Calling wait on a already deallocated device_context!");
}

} // namespace zoo::render
