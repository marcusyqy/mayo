#include "frame_memory_pool.hpp"

namespace zoo::render::resources {

frame_memory_pool(std::shared_ptr<device_context> context, size_t size) noexcept
    : fixed_size_(size) {}

~frame_memory_pool() noexcept;

memory_view allocate() noexcept;

} // namespace zoo::render::resources
