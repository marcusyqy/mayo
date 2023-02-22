#pragma once

#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "render/utils/box.hpp"
#include "vma/vk_mem_alloc.h"


namespace zoo::render::resources {

struct memory_region {
    size_t start_offset{};
    size_t end_offset{};
};

class memory_view;

class memory : utils::box<VkDeviceMemory> {
public:
    using underlying_type = utils::box<VkDeviceMemory>;

    using value_type = underlying_type::value_type;
    using underlying_type::operator value_type;
    using underlying_type::operator bool;

    memory(std::shared_ptr<device_context> context, size_t size) noexcept;

    [[nodiscard]] memory_region region() const noexcept;
    [[nodiscard]] memory_view view(const memory_region& rg) const noexcept;
    [[nodiscard]] memory_view view() const noexcept;
    [[nodiscard]] operator memory_view() const noexcept;

    void reset() noexcept;

private:
    size_t size_;
};

class memory_view {
public:
    using view_type = VkDeviceMemory;

    memory_view(const memory& memory, const memory_region& region);

private:
    view_type mem_;
};

} // namespace zoo::render::resources
