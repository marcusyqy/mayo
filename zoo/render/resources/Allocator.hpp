#include "vma/vk_mem_alloc.h"
#include "Buffer.hpp"

namespace zoo::render::resources {

class Allocator {
public:
    Allocator() noexcept;
    ~Allocator() noexcept;

    Allocator(const Allocator& other) = delete;
    Allocator& operator=(const Allocator& other) = delete;

    Allocator(Allocator&& other) noexcept = delete;
    Allocator& operator=(Allocator&& other) noexcept = delete;

    void emplace(
        VkInstance instance, VkDevice device, VkPhysicalDevice pd) noexcept;

    void reset() noexcept;

    operator bool() noexcept { return underlying_ != nullptr; }

    Buffer allocate_buffer(size_t size, VkBufferUsageFlags usage) noexcept;

private:
    VmaAllocator underlying_;
};

} // namespace zoo::render::resources
