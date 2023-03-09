#pragma once

#include "render/DeviceContext.hpp"
#include "render/fwd.hpp"
#include "vma/vk_mem_alloc.h"

namespace zoo::render::resources {

struct MemoryRegion {
    size_t start_offset{};
    size_t end_offset{};
};

class MemoryView;

class Memory {
public:
    using underlying_type = VkDeviceMemory;

    Memory(DeviceContext& context, size_t size) noexcept;

    [[nodiscard]] MemoryRegion region() const noexcept;
    [[nodiscard]] MemoryView view(const MemoryRegion& rg) const noexcept;
    [[nodiscard]] MemoryView view() const noexcept;
    [[nodiscard]] operator MemoryView() const noexcept;

    void reset() noexcept;

private:
    DeviceContext* context_;
    VkDeviceMemory memory_;
    size_t size_;
};

class MemoryView {
public:
    using view_type = VkDeviceMemory;

    MemoryView(const Memory& memory, const MemoryRegion& region);

private:
    view_type mem_;
};

} // namespace zoo::render::resources
