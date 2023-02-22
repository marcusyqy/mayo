#pragma once

#include "render/DeviceContext.hpp"
#include "render/fwd.hpp"
#include "render/utils/Box.hpp"
#include "vma/vk_mem_alloc.h"

namespace zoo::render::resources {

struct MemoryRegion {
    size_t start_offset{};
    size_t end_offset{};
};

class MemoryView;

class Memory : utils::Box<VkDeviceMemory> {
public:
    using underlying_type = utils::Box<VkDeviceMemory>;

    using value_type = underlying_type::value_type;
    using underlying_type::operator value_type;
    using underlying_type::operator bool;

    Memory(std::shared_ptr<DeviceContext> context, size_t size) noexcept;

    [[nodiscard]] MemoryRegion region() const noexcept;
    [[nodiscard]] MemoryView view(const MemoryRegion& rg) const noexcept;
    [[nodiscard]] MemoryView view() const noexcept;
    [[nodiscard]] operator MemoryView() const noexcept;

    void reset() noexcept;

private:
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
