#include "Allocator.hpp"
#include "render/Defines.hpp"
#include "render/fwd.hpp"

namespace zoo::render::resources {

Allocator::Allocator() noexcept : underlying_{ nullptr } {}

Allocator::~Allocator() noexcept {
    if (underlying_ != nullptr)
        reset();
}

void Allocator::emplace(
    VkInstance instance, VkDevice device, VkPhysicalDevice pd) noexcept {
    create_info_ = VmaAllocatorCreateInfo{};
    create_info_.vulkanApiVersion = Defines::vk_version;
    create_info_.physicalDevice = pd;
    create_info_.device = device;
    create_info_.instance = instance;
    VK_EXPECT_SUCCESS(vmaCreateAllocator(&create_info_, &underlying_));
}

void Allocator::reset() noexcept {
    if (underlying_ != nullptr) {
        vmaDestroyAllocator(underlying_);
        underlying_ = nullptr;
    }
}

} // namespace zoo::render::resources
