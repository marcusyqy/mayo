
#pragma once
#include "render/DeviceContext.hpp"
#include "render/fwd.hpp"
#include "render/utils/Box.hpp"

namespace zoo::render::resources {

class Buffer : utils::Box<VkBuffer> {
public:
    using underlying_type = utils::Box<VkBuffer>;
    Buffer(DeviceContext& context, size_t size,
        VkBufferUsageFlags usage, VkSharingMode sharing_mode) noexcept;
};

} // namespace zoo::render::resources
