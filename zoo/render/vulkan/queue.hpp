#pragma once

#include "utils/queue_family_properties.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

class queue {
public:
    queue(utils::queue_family_properties family_props) noexcept;

private:
    VkQueue queue_;
};
} // namespace zoo::render::vulkan
