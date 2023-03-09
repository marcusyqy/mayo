#include "Fence.hpp"
#include "render/DeviceContext.hpp"

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

void Fence::reset() noexcept { vkResetFences(*context_, 1, &underlying_); }

void Fence::wait() noexcept {
    vkWaitForFences(*context_, 1, &underlying_, VK_TRUE,
        std::numeric_limits<uint64_t>::max());
}

Fence::Fence(DeviceContext& context) noexcept
    : context_(std::addressof(context)), underlying_(create_fence(context)) {}

Fence::~Fence() noexcept {
    if (context_ != nullptr) {
        context_->release_device_resource(underlying_);
        context_ = nullptr;
        underlying_ = nullptr;
    }
}

Fence::Fence(Fence&& other) noexcept
    : context_(std::move(other.context_)),
      underlying_(std::move(other.underlying_)) {
    other.context_ = nullptr;
    other.underlying_ = nullptr;
}

Fence& Fence::operator=(Fence&& other) noexcept {
    std::swap(context_, other.context_);
    std::swap(underlying_, other.underlying_);
    return *this;
}
} // namespace zoo::render::sync
