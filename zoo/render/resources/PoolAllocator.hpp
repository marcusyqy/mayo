#pragma once
#include "Allocator.hpp"
#include "vma/vk_mem_alloc.h"

namespace zoo::render::resources {

class PoolAllocator {
public:
    PoolAllocator(const Allocator& allocator) noexcept;
    ~PoolAllocator() noexcept;

    PoolAllocator(const PoolAllocator& other) = delete;
    PoolAllocator& operator=(const PoolAllocator& other) = delete;

    PoolAllocator(PoolAllocator&& other) noexcept = delete;
    PoolAllocator& operator=(PoolAllocator&& other) noexcept = delete;

private:
    VmaPoolCreateInfo create_info_;
    VmaPool underlying_;
};

} // namespace zoo::render::resources
