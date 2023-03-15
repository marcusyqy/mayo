#pragma once
#include "render/fwd.hpp"

namespace zoo::render::resources {

class Buffer {
public:
    Buffer(VkBuffer buffer, VkBufferUsageFlags usage, size_t size,
        VmaAllocation allocation, VmaAllocationInfo allocation_info) noexcept;
    ~Buffer() noexcept;

private:
    VkBuffer buffer_;
    VkBufferUsageFlags usage_;
    size_t size_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
};

} // namespace zoo::render::resources
