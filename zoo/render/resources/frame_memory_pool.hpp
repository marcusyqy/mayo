#pragma once
#include "render/fwd.hpp"

#include "memory.hpp"
#include "render/device_context.hpp"

namespace zoo::render::resources {

class frame_memory_pool {
public:
    frame_memory_pool(std::shared_ptr<device_context> context, size_t size) noexcept;
    ~frame_memory_pool() noexcept;

    memory_view allocate() noexcept;

private:
    const size_t fixed_size_;
    memory mem_;
};

} // namespace zoo::render::resources
