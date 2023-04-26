#pragma once
#include "zoo.hpp"

#include "utils/PhysicalDevice.hpp"

#include "core/platform/Query.hpp"
#include "fwd.hpp"
#include "render/resources/Allocator.hpp"
#include <memory>

namespace zoo::render {

class DeviceContext {
public:
    DeviceContext(VkInstance instance, utils::PhysicalDevice pdevice,
        const utils::QueueFamilyProperties& family_props,
        const platform::render::Query& query) noexcept;

    ~DeviceContext() noexcept;

    DeviceContext(const DeviceContext& other) noexcept = delete;
    DeviceContext(DeviceContext&& other) noexcept = delete;

    DeviceContext& operator=(const DeviceContext& other) noexcept = delete;
    DeviceContext& operator=(DeviceContext&& other) noexcept = delete;

    void reset() noexcept;

    operator const utils::PhysicalDevice&() const noexcept {
        return physical();
    }

    const utils::PhysicalDevice& physical() const noexcept { return physical_; }

    operator const VkDevice&() const noexcept { return logical(); }
    const VkDevice& logical() const noexcept { return logical_; }

    VkCommandBuffer vk_command_buffer_from_pool() const noexcept;

    VkQueue retrieve(Operation op) const noexcept;

    // release resource
    void release_device_resource(VkFence fence) noexcept;
    void release_device_resource(VkRenderPass renderpass) noexcept;
    void release_device_resource(VkSemaphore semaphore) noexcept;
    void release_device_resource(VkBuffer buffer) noexcept;
    void release_device_resource(VkDeviceMemory device_memory) noexcept;

    void wait() noexcept;

    resources::Allocator& allocator() noexcept { return allocator_; }

    const resources::Allocator& allocator() const noexcept {
        return allocator_;
    }

private:
    utils::PhysicalDevice physical_ = nullptr;
    VkDevice logical_ = nullptr;
    VkQueue queue_ = nullptr;
    utils::QueueFamilyProperties queue_properties_;
    VkCommandPool command_pool_ = nullptr;

    resources::Allocator allocator_;
};

} // namespace zoo::render
