
#include "InstanceBuilder.hpp"

namespace zoo::render::vulkan::utils {

VkInstance InstanceBuilder::build() noexcept {
    VkResult result = vkCreateInstance(&create_info_, nullptr, &instance_);
    if (result != VK_SUCCESS) {
        // report results here
    }
    return instance_;
}

} // namespace zoo::render::vulkan::utils