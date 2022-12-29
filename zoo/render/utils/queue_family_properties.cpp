#include "queue_family_properties.hpp"

namespace zoo::render::utils {

queue_family_properties::queue_family_properties(
    uint32_t queue_index, VkQueueFamilyProperties properties) noexcept :
    queue_index_(queue_index),
    properties_(properties) {}

bool queue_family_properties::has_compute() const noexcept {
    return properties_.queueFlags & VK_QUEUE_COMPUTE_BIT;
}

bool queue_family_properties::has_graphics() const noexcept {
    return properties_.queueFlags & VK_QUEUE_GRAPHICS_BIT;
}

bool queue_family_properties::has_transfer() const noexcept {
    return properties_.queueFlags & VK_QUEUE_TRANSFER_BIT;
}

bool queue_family_properties::has_protected() const noexcept {
    return properties_.queueFlags & VK_QUEUE_PROTECTED_BIT;
}

bool queue_family_properties::has_sparse_binding() const noexcept {
    return properties_.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
}

bool queue_family_properties::valid() const noexcept {
    return properties_.queueCount > 0;
}

queue_family_properties::size_type
queue_family_properties::size() const noexcept {
    return properties_.queueCount;
}

} // namespace zoo::render::utils
