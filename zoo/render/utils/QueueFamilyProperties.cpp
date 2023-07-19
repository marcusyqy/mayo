#include "QueueFamilyProperties.hpp"

namespace zoo::render::utils {

QueueFamilyProperties::QueueFamilyProperties(uint32_t queue_index, VkQueueFamilyProperties properties) noexcept :
    queue_index_(queue_index), properties_(properties) {}

bool QueueFamilyProperties::has_compute() const noexcept { return properties_.queueFlags & VK_QUEUE_COMPUTE_BIT; }

bool QueueFamilyProperties::has_graphics() const noexcept { return properties_.queueFlags & VK_QUEUE_GRAPHICS_BIT; }

bool QueueFamilyProperties::has_transfer() const noexcept { return properties_.queueFlags & VK_QUEUE_TRANSFER_BIT; }

bool QueueFamilyProperties::has_protected() const noexcept { return properties_.queueFlags & VK_QUEUE_PROTECTED_BIT; }

bool QueueFamilyProperties::has_sparse_binding() const noexcept {
    return properties_.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
}

bool QueueFamilyProperties ::valid() const noexcept { return properties_.queueCount > 0; }

QueueFamilyProperties::size_type QueueFamilyProperties::size() const noexcept { return properties_.queueCount; }

} // namespace zoo::render::utils
