#include "Buffer.hpp"

namespace zoo::render::resources {

namespace buffer {

Buffer Builder::build() noexcept {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size_;
    buffer_info.usage = usage_;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = memory_usage_;

    VK_EXPECT_SUCCESS(vmaCreateBuffer(allocator_, &buffer_info, &alloc_info,
        &buffer_, &allocation_, &allocation_info_));

    return Buffer(buffer_, usage_, size_, allocator_, allocation_, allocation_info_);
}

Builder& Builder::size(size_t size) noexcept {
    size_ = size;
    return *this;
}

Builder& Builder::usage(VkBufferUsageFlags usage) noexcept {
    usage_ = usage;
    return *this;
}

Builder& Builder::allocation_type(VmaMemoryUsage usage) noexcept {
    memory_usage_ = usage;
    return *this;
}

Builder& Builder::allocation_flag(VmaAllocationCreateFlags flags) noexcept {
    memory_flags_ = flags;
    return *this;
}

Builder::Builder(const Allocator& allocator) noexcept {
    allocator_ = allocator;
}

} // namespace buffer

Buffer::Buffer(VkBuffer buffer, VkBufferUsageFlags usage, size_t size,
    VmaAllocator allocator, VmaAllocation allocation,
    VmaAllocationInfo allocation_info) noexcept
    : buffer_{buffer}, usage_{usage}, size_{size}, allocator_{allocator},
      allocation_{allocation}, allocation_info_{allocation_info} {}

Buffer::~Buffer() noexcept {
    vmaDestroyBuffer(allocator_, buffer_, allocation_);
}

buffer::Builder Buffer::start_build(const Allocator& allocator) noexcept {
    return {allocator};
}

void* Buffer::map() noexcept {
    void* data = nullptr;
    VK_EXPECT_SUCCESS(vmaMapMemory(allocator_, allocation_, &data));
    return data;
}

void Buffer::unmap() noexcept { vmaUnmapMemory(allocator_, allocation_); }

void Buffer::map(stdx::function_ref<void(void*)> fn) noexcept {
    fn(map());
    unmap();
}

VkBuffer Buffer::handle() const noexcept { return buffer_; }

// TODO: find out if there is anything to do for buffer;
VkDeviceSize Buffer::offset() const noexcept { return 0; }

} // namespace zoo::render::resources
