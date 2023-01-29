
#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "render/utils/box.hpp"

namespace zoo::render::resources {

class buffer {
public:
    using buffer_handle_t = utils::box<VkBuffer>;
    using memory_handle_t = utils::box<VkDeviceMemory>;
    buffer(std::shared_ptr<device_context> context, size_t size,
        VkBufferUsageFlags usage, VkSharingMode sharing_mode) noexcept;

private:
    buffer_handle_t buffer_;
    memory_handle_t memory_;
};

} // namespace zoo::render::resources
