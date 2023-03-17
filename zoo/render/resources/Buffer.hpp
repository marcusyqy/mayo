#pragma once
#include "Allocator.hpp"
#include "render/fwd.hpp"

#include <stdx/function_ref.hpp>

namespace zoo::render::resources {

class Buffer;

namespace buffer {

class Builder {
public:
    Buffer build() noexcept;

    Builder(const Allocator& allocator) noexcept;
    Builder& size(size_t size) noexcept;
    Builder& usage(VkBufferUsageFlags usage) noexcept;
    Builder& allocation_type(VmaMemoryUsage usage) noexcept;

private:
    VkBuffer buffer_ = {};
    VkBufferUsageFlags usage_ = {};
    size_t size_ = {};
    VmaAllocator allocator_ = {};
    VmaAllocation allocation_ = {};
    VmaAllocationInfo allocation_info_ = {};
    VmaMemoryUsage memory_usage_ = VMA_MEMORY_USAGE_AUTO;
};

} // namespace buffer

// IDEA: create a buffer descriptor here.

class Buffer {
public:
    using builder_type = buffer::Builder;

    static builder_type start_build(const Allocator& allocator) noexcept;

    Buffer(VkBuffer buffer, VkBufferUsageFlags usage, size_t size,
        VmaAllocator allocator, VmaAllocation allocation,
        VmaAllocationInfo allocation_info) noexcept;

    ~Buffer() noexcept;

    void* map() noexcept;
    void unmap() noexcept;

    void map(stdx::function_ref<void(void*)> fn) noexcept;

    VkBuffer handle() const noexcept;
    VkDeviceSize offset() const noexcept;

private:
    VkBuffer buffer_;
    VkBufferUsageFlags usage_;
    size_t size_;
    VmaAllocator allocator_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
};

} // namespace zoo::render::resources
