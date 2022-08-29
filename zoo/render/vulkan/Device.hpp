#pragma once
#include <memory>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

class Device : std::enable_shared_from_this<Device> {
public:
    Device(VkInstance instance) noexcept;
    ~Device() noexcept;

    Device(const Device& other) noexcept = delete;
    Device(Device&& other) noexcept = delete;

    Device& operator=(const Device& other) noexcept = delete;
    Device& operator=(Device&& other) noexcept = delete;

    void cleanup() noexcept;
    void release_device_resource(VkFence fence);

private:
    VkPhysicalDevice physical_ = VK_NULL_HANDLE;
    VkDevice logical_ = VK_NULL_HANDLE;
};

} // namespace zoo::render::vulkan