#include "fence.hpp"

namespace zoo::render::sync {

VkFence create_fence(VkDevice device) noexcept {
    VkFenceCreateInfo fence_info{};
    VkFence fence_obj{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // TODO: figure out if we really need to signal at the start.
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_EXPECT_SUCCESS(
        vkCreateFence(device, &fence_info, nullptr, std::addressof(fence_obj)));
    return fence_obj;
}

fence::fence(std::shared_ptr<device_context> context) noexcept
    : underlying_type(context, create_fence(*context)) {}

void fence::reset() noexcept { vkResetFences(*context_, 1, &type_); }

void fence::wait() noexcept {
    vkWaitForFences(
        *context_, 1, &type_, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

} // namespace zoo::render::sync