#pragma once

#include "core/fwd.hpp"
#include "render/fwd.hpp"

namespace zoo::render::utils {

class Queue_Family_Properties {
public:
    using size_type = u32;

    Queue_Family_Properties(uint32_t queue_index, VkQueueFamilyProperties properties) noexcept;

    [[nodiscard]] bool has_compute() const noexcept;
    [[nodiscard]] bool has_graphics() const noexcept;
    [[nodiscard]] bool has_transfer() const noexcept;
    [[nodiscard]] bool has_protected() const noexcept;
    [[nodiscard]] bool has_sparse_binding() const noexcept;

    [[nodiscard]] bool valid() const noexcept;
    size_type size() const noexcept;

    uint32_t index() const noexcept { return queue_index_; }

private:
    uint32_t queue_index_;
    VkQueueFamilyProperties properties_;
};

} // namespace zoo::render::utils
