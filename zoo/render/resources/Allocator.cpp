#include "Allocator.hpp"
#include "render/fwd.hpp"
#include "render/Defines.hpp"

namespace zoo::render::resources {

Allocator::Allocator() noexcept : underlying_{nullptr} {}

Allocator::~Allocator() noexcept {
    if (underlying_ != nullptr)
        reset();
}

void Allocator::emplace(
    VkInstance instance, VkDevice device, VkPhysicalDevice pd) noexcept {
    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.vulkanApiVersion = Defines::vk_version;
    allocator_create_info.physicalDevice = pd;
    allocator_create_info.device = device;
    allocator_create_info.instance = instance;
    VK_EXPECT_SUCCESS(vmaCreateAllocator(&allocator_create_info, &underlying_));
}

void Allocator::reset() noexcept {
    if (underlying_ != nullptr) {
        vmaDestroyAllocator(underlying_);
        underlying_ = nullptr;
    }
}

} // namespace zoo::render::resources
