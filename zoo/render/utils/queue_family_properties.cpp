#include "Queue_Family_Properties.hpp"

namespace zoo::render::utils {

Queue_Family_Properties::Queue_Family_Properties(uint32_t queue_index, VkQueueFamilyProperties properties) noexcept :
    queue_index_(queue_index), properties_(properties) {}

bool Queue_Family_Properties::has_compute() const noexcept { return properties_.queueFlags & VK_QUEUE_COMPUTE_BIT; }

bool Queue_Family_Properties::has_graphics() const noexcept { return properties_.queueFlags & VK_QUEUE_GRAPHICS_BIT; }

bool Queue_Family_Properties::has_transfer() const noexcept { return properties_.queueFlags & VK_QUEUE_TRANSFER_BIT; }

bool Queue_Family_Properties::has_protected() const noexcept { return properties_.queueFlags & VK_QUEUE_PROTECTED_BIT; }

bool Queue_Family_Properties::has_sparse_binding() const noexcept {
    return properties_.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
}

bool Queue_Family_Properties ::valid() const noexcept { return properties_.queueCount > 0; }

Queue_Family_Properties::size_type Queue_Family_Properties::size() const noexcept { return properties_.queueCount; }

} // namespace zoo::render::utils
