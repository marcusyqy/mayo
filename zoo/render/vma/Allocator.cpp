#include "Allocator.hpp"

namespace zoo::render::vma {

Allocator::Allocator() noexcept : underlying_{nullptr} {}

Allocator::~Allocator() noexcept {
    if (underlying_ != nullptr) {
        reset();
    }
}

void Allocator::emplace(
    VkInstance instance, VkDevice device, VkPhysicalDevice pd) noexcept {
    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_2;
    allocator_create_info.physicalDevice = pd;
    allocator_create_info.device = device;
    allocator_create_info.instance = instance;
    allocator_create_info.pVulkanFunctions = nullptr;
    vmaCreateAllocator(&allocator_create_info, &underlying_);
}

void Allocator::reset() noexcept {
    vmaDestroyAllocator(underlying_);
    underlying_ = nullptr;
}
} // namespace zoo::render::vma
