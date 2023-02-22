#include "memory.hpp"

namespace zoo::render::resources {

namespace {

VkDeviceMemory allocate_memory(
    VkPhysicalDevice pd, VkDevice device, size_t size) noexcept {

    VkPhysicalDeviceMemoryProperties mem_props{};
    vkGetPhysicalDeviceMemoryProperties(pd, &mem_props);

    VkMemoryAllocateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    create_info.pNext = nullptr;
    create_info.allocationSize = size;

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.pNext = nullptr;
    bufferInfo.size = 65536;
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VmaAllocator allocator{};
    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(
        allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);

    VkDeviceMemory device_memory{};
    VK_EXPECT_SUCCESS(
        vkAllocateMemory(device, &create_info, nullptr, &device_memory));
    return device_memory;
}
} // namespace
  //

memory::memory(std::shared_ptr<device_context> context, size_t size) noexcept
    : underlying_type(
          context, allocate_memory(context->physical(), *context, size)),
      size_(size) {}

memory_region memory::region() const noexcept { return {0, size_}; }
memory_view memory::view() const noexcept { return {*this, region()}; }
memory_view memory::view(const memory_region& rg) const noexcept {
    return {*this, rg};
}
memory::operator memory_view() const noexcept { return view(); }

void memory::reset() noexcept {
    underlying_type::reset();
    size_ = 0;
}

memory_view::memory_view(const memory& memory, const memory_region& region) {
    std::ignore = memory;
    std::ignore = region;
}
} // namespace zoo::render::resources
