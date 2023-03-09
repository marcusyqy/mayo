#include "Memory.hpp"

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
    bufferInfo.size = size;
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;

    VmaAllocator allocator{};
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo alloc_info{};
    vmaCreateBuffer(allocator, &bufferInfo, &alloc_create_info, &buffer,
        &allocation, &alloc_info);

    VkDeviceMemory device_memory{};
    VK_EXPECT_SUCCESS(
        vkAllocateMemory(device, &create_info, nullptr, &device_memory));
    return device_memory;
}
} // namespace
  //

Memory::Memory(DeviceContext& context, size_t size) noexcept
    : context_{std::addressof(context)}, memory_{allocate_memory(
                                             context.physical(), context,
                                             size)},
      size_(size) {}

MemoryRegion Memory::region() const noexcept { return {0, size_}; }
MemoryView Memory::view() const noexcept { return {*this, region()}; }
MemoryView Memory::view(const MemoryRegion& rg) const noexcept {
    return {*this, rg};
}
Memory::operator MemoryView() const noexcept { return view(); }

void Memory::reset() noexcept { size_ = 0; }

MemoryView::MemoryView(const Memory& memory, const MemoryRegion& region) {
    (void)memory;
    (void)region;
}
} // namespace zoo::render::resources
