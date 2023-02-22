#pragma once

#include "render/fwd.hpp"

namespace zoo::render::utils {

class queue_family_properties {
public:
    using size_type = uint32_t;

    queue_family_properties(
        uint32_t queue_index, VkQueueFamilyProperties properties) noexcept;

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
