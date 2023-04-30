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

    Builder(const Allocator& allocator, std::string_view name) noexcept;
    Builder& size(size_t size) noexcept;
    Builder& usage(VkBufferUsageFlags usage) noexcept;
    Builder& allocation_type(VmaMemoryUsage usage) noexcept;
    Builder& allocation_flag(VmaAllocationCreateFlags flags) noexcept;

private:
    VkBuffer buffer_ = {};
    VkBufferUsageFlags usage_ = {};
    size_t size_ = {};
    VmaAllocator allocator_ = {};
    VmaAllocation allocation_ = {};
    VmaAllocationInfo allocation_info_ = {};
    VmaMemoryUsage memory_usage_ = VMA_MEMORY_USAGE_AUTO;
    VmaAllocationCreateFlags memory_flags_ = {};
    std::string name_ = {};
};

} // namespace buffer

// IDEA: create a buffer descriptor here.

class Buffer {
public:
    using builder_type = buffer::Builder;

    static builder_type start_build(const Allocator& allocator, std::string_view name) noexcept;

    explicit Buffer(std::string name, VkBuffer buffer, VkBufferUsageFlags usage, size_t size,
        VmaAllocator allocator, VmaAllocation allocation,
        VmaAllocationInfo allocation_info) noexcept;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    ~Buffer() noexcept;

    void* map() noexcept;
    void unmap() noexcept;

    void map(stdx::function_ref<void(void*)> fn) noexcept;

    template<typename Type>
    void map(stdx::function_ref<void(Type*)> fn) noexcept {
        fn(map<Type>());
        unmap();
    }

    template<typename Type>
    Type* map() noexcept {
        return reinterpret_cast<Type*>(map());
    }

    VkBuffer handle() const noexcept;
    VkDeviceSize offset() const noexcept;

private:
    std::string name_;

    VkBuffer buffer_;
    VkBufferUsageFlags usage_;
    size_t size_;
    VmaAllocator allocator_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
};

} // namespace zoo::render::resources
