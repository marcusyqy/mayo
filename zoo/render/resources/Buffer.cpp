#include "Buffer.hpp"

namespace zoo::render::resources {

namespace buffer {

Buffer Builder::build(const Allocator& allocator) noexcept {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = count_ * obj_size_;
    buffer_info.usage = usage_;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = memory_usage_;

    VmaAllocation allocation{};
    VK_EXPECT_SUCCESS(vmaCreateBuffer(allocator, &buffer_info, &alloc_info,
        &buffer_, &allocation, &allocation_info_));

    return Buffer(name_, buffer_, usage_, obj_size_, count_, allocator,
        allocation, allocation_info_);
}

Builder& Builder::count(size_t count) noexcept {
    count_ = count;
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

Builder::Builder(std::string_view name, size_t size) noexcept {
    name_ = name;
    obj_size_ = size;
}

} // namespace buffer

Buffer::Buffer(std::string name, VkBuffer buffer, VkBufferUsageFlags usage,
    size_t obj_size, size_t count, VmaAllocator allocator,
    VmaAllocation allocation, VmaAllocationInfo allocation_info) noexcept
    : name_{ name }, buffer_{ buffer }, usage_{ usage }, obj_size_{ obj_size },
      count_{ count }, allocator_{ allocator }, allocation_{ allocation },
      allocation_info_{ allocation_info } {}

Buffer::~Buffer() noexcept {
    vmaDestroyBuffer(allocator_, buffer_, allocation_);
}

buffer::Builder Buffer::start_build(
    std::string_view name, size_t size) noexcept {
    return { name, size };
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
