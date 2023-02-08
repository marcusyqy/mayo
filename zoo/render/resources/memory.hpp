
#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "render/utils/box.hpp"

namespace zoo::render::resources {

class memory : utils::box<VkDeviceMemory> {
public:
    using underlying_type = utils::box<VkDeviceMemory>;

private:
};

class memory_view {
public:
    using view_type = VkDeviceMemory;

private:
    view_type memory_;
};

} // namespace zoo::render::resources