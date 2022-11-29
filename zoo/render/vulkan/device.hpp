#pragma once
#include <memory>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

class device : std::enable_shared_from_this<device> {
public:
    device(VkInstance instance, VkPhysicalDevice physical_device) noexcept;
    ~device() noexcept;

    device(const device& other) noexcept = delete;
    device(device&& other) noexcept = delete;

    void operator=(const device& other) noexcept = delete;
    void operator=(device&& other) noexcept = delete;
    void reset() noexcept;
    void release_device_resource(VkFence fence) noexcept;

private:
    VkPhysicalDevice physical_ = VK_NULL_HANDLE;
    VkDevice logical_ = VK_NULL_HANDLE;
};

} // namespace zoo::render::vulkan
