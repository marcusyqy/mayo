#include "allocator.hpp"

namespace zoo::render::vma {

allocator::allocator() noexcept : underlying_{nullptr} {}

allocator::~allocator() noexcept {
    if (underlying_ != nullptr) {
        reset();
    }
}

void allocator::emplace(
    VkInstance instance, VkDevice device, VkPhysicalDevice pd) noexcept {
    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_2;
    allocator_create_info.physicalDevice = pd;
    allocator_create_info.device = device;
    allocator_create_info.instance = instance;
    allocator_create_info.pVulkanFunctions = nullptr;
    vmaCreateAllocator(&allocator_create_info, &underlying_);
}

void allocator::reset() noexcept {
    vmaDestroyAllocator(underlying_);
    underlying_ = nullptr;
}
} // namespace zoo::render::resources
