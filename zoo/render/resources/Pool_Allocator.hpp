#pragma once
#include "Allocator.hpp"
#include "vma/vk_mem_alloc.h"

namespace zoo::render::resources {

class Pool_Allocator {
public:
    Pool_Allocator(const Allocator& allocator) noexcept;
    ~Pool_Allocator() noexcept;

    Pool_Allocator(const Pool_Allocator& other)            = delete;
    Pool_Allocator& operator=(const Pool_Allocator& other) = delete;

    Pool_Allocator(Pool_Allocator&& other) noexcept            = delete;
    Pool_Allocator& operator=(Pool_Allocator&& other) noexcept = delete;

private:
    VmaPoolCreateInfo create_info_;
    VmaPool underlying_;
};

} // namespace zoo::render::resources
