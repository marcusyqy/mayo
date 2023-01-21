
#include "semaphore.hpp"

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

semaphore::semaphore(std::shared_ptr<device_context> context) noexcept
    : underlying_type{context, create_semaphore(*context)} {}

} // namespace zoo::render::sync
