#include "Semaphore.hpp"
#include "render/DeviceContext.hpp"

namespace zoo::render::sync {

namespace {

VkSemaphore create_semaphore(VkDevice device) noexcept {
    VkSemaphoreCreateInfo semaphore_info{};
    VkSemaphore semaphore_obj{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_EXPECT_SUCCESS(vkCreateSemaphore(
        device, &semaphore_info, nullptr, std::addressof(semaphore_obj)));
    return semaphore_obj;
}

} // namespace

Semaphore::Semaphore(DeviceContext& context) noexcept
    : context_{std::addressof(context)},
      underlying_(create_semaphore(context)) {}

Semaphore::~Semaphore() noexcept {
    if (context_ != nullptr) {
        context_->release_device_resource(underlying_);
        underlying_ = nullptr;
        context_ = nullptr;
    }
}

Semaphore::Semaphore(Semaphore&& other) noexcept
    : context_(std::move(other.context_)),
      underlying_(std::move(other.underlying_)) {
    other.context_ = nullptr;
    other.underlying_ = nullptr;
}

Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
    std::swap(context_, other.context_);
    std::swap(underlying_, other.underlying_);
    return *this;
}

} // namespace zoo::render::sync
