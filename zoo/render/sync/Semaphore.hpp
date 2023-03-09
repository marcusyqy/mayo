#pragma once

#include "render/fwd.hpp"

namespace zoo::render::sync {

class Semaphore {
public:
    using underlying_type = VkSemaphore;

    underlying_type get() const noexcept { return underlying_; }
    operator underlying_type() const noexcept { return get(); }

    Semaphore(DeviceContext& context) noexcept;
    ~Semaphore() noexcept;

    Semaphore(const Semaphore& other) = delete;
    Semaphore& operator=(const Semaphore& other) = delete;

    Semaphore(Semaphore&& other) noexcept;
    Semaphore& operator=(Semaphore&& other) noexcept;

private:
    DeviceContext* context_;
    VkSemaphore underlying_;
};
} // namespace zoo::render::sync
