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

    Fence() noexcept = default;
    Fence(DeviceContext& context, bool signaled = false) noexcept;
    ~Fence() noexcept;

    Fence(const Fence& other)            = delete;
    Fence& operator=(const Fence& other) = delete;

    Fence(Fence&& other) noexcept;
    Fence& operator=(Fence&& other) noexcept;

    enum Status { signaled, unsignaled, error };
    Status is_signaled() const noexcept;

    bool valid() const noexcept;

private:
    DeviceContext* context_     = nullptr;
    underlying_type underlying_ = VK_NULL_HANDLE;
};

} // namespace zoo::render::sync
