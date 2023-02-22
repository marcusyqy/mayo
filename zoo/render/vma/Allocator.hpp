#include "vma/vk_mem_alloc.h"

namespace zoo::render::vma {

class allocator {
public:
    allocator() noexcept;
    ~allocator() noexcept;

    allocator(const allocator& other) = delete;
    allocator& operator=(const allocator& other) = delete;

    allocator(allocator&& other) noexcept = delete;
    allocator& operator=(allocator&& other) noexcept = delete;

    void emplace(
        VkInstance instance, VkDevice device, VkPhysicalDevice pd) noexcept;

    void reset() noexcept;
    operator bool() noexcept { return underlying_ != nullptr; }

private:
    VmaAllocator underlying_;
};

} // namespace zoo::render::resources
