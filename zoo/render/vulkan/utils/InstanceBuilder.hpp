#pragma once
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan::utils {

class InstanceBuilder {
public:
    [[nodiscard("built instance cannot be discarded!")]] VkInstance
    build() noexcept;

private:
    VkInstanceCreateInfo create_info_ = {};
    VkInstance instance_ = VK_NULL_HANDLE;
};

} // namespace zoo::render::vulkan::utils