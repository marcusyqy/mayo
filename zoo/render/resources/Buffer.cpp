#include "Buffer.hpp"

namespace zoo::render::resources {

Buffer::Buffer(VkBuffer buffer, VkBufferUsageFlags usage, size_t size,
    VmaAllocation allocation, VmaAllocationInfo allocation_info) noexcept
    : buffer_{buffer}, usage_{usage}, size_{size}, allocation_{allocation},
      allocation_info_{allocation_info} {}
Buffer::~Buffer() noexcept {}

} // namespace zoo::render::resources
