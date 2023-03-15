#include "Allocator.hpp"
#include "render/fwd.hpp"

namespace zoo::render::resources {
namespace {
// TODO: Unify all these
const auto VK_VERSION = VK_API_VERSION_1_3;
} // namespace

Allocator::Allocator() noexcept : underlying_{nullptr} {}

Allocator::~Allocator() noexcept {
    if (underlying_ != nullptr) {
        reset();
    }
}

void Allocator::emplace(
    VkInstance instance, VkDevice device, VkPhysicalDevice pd) noexcept {
    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.vulkanApiVersion = VK_VERSION;
    allocator_create_info.physicalDevice = pd;
    allocator_create_info.device = device;
    allocator_create_info.instance = instance;
    allocator_create_info.pVulkanFunctions = nullptr;
    VK_EXPECT_SUCCESS(vmaCreateAllocator(&allocator_create_info, &underlying_));
}

void Allocator::reset() noexcept {
    vmaDestroyAllocator(underlying_);
    underlying_ = nullptr;
}

Buffer Allocator::allocate_buffer(
    size_t size, VkBufferCreateFlags usage) noexcept {
    VkBuffer buffer{};

    VkBufferCreateInfo buffer_info;
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VK_EXPECT_SUCCESS(vmaCreateBuffer(
        underlying_, &buffer_info, &alloc_info, &buffer, &allocation, &info));

    return Buffer{buffer, usage, size, allocation, info};
}

} // namespace zoo::render::resources
