#include "vma/vk_mem_alloc.h"

namespace zoo::render::vma {

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

private:
    VmaAllocator underlying_;
};

} // namespace zoo::render::vma
