
#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "render/utils/box.hpp"

namespace zoo::render::resources {

class buffer : utils::box<VkBuffer> {
public:
    using underlying_type = utils::box<VkBuffer>;
    buffer(std::shared_ptr<device_context> context, size_t size,
        VkBufferUsageFlags usage, VkSharingMode sharing_mode) noexcept;
};

} // namespace zoo::render::resources
