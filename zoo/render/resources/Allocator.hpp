#pragma once
#include "vma/vk_mem_alloc.h"

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

    VmaAllocator get() const noexcept { return underlying_; }
    operator VmaAllocator() const noexcept { return get(); };

private:
    VmaAllocator underlying_;
};

} // namespace zoo::render::resources
