#pragma once

#include "render/fwd.hpp"

namespace zoo::render::sync {

class Fence {
public:
    using underlying_type = VkFence;

    void reset() noexcept;
    void wait() noexcept;

    underlying_type get() const noexcept { return underlying_; }
    operator underlying_type() const noexcept { return get(); }

    Fence(DeviceContext& context) noexcept;
    ~Fence() noexcept;

    Fence(const Fence& other) = delete;
    Fence& operator=(const Fence& other) = delete;

    Fence(Fence&& other) noexcept;
    Fence& operator=(Fence&& other) noexcept;

private:
    DeviceContext* context_;
    underlying_type underlying_;
};

} // namespace zoo::render::sync
