#include "buffer.hpp"

namespace zoo::render::resources {
namespace {

VkBuffer create_buffer(VkDevice device, size_t size, VkBufferUsageFlags usage,
    VkSharingMode sharing_mode) noexcept {
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = sharing_mode;

    VkBuffer buffer{};
    VK_EXPECT_SUCCESS(vkCreateBuffer(device, &create_info, nullptr, &buffer));
    return buffer;
}

VkDeviceMemory allocate_memory(VkDevice device, VkBuffer buffer, size_t size, VkBufferUsageFlags usage,
    VkSharingMode sharing_mode) noexcept {
    VkDeviceBufferMemoryRequirements requirements;
    vkGetDeviceBufferMemoryRequirements(device, &device_requirements_info, &memory_requirements);

    VkMemoryAllocateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    VkDeviceMemory device_memory{};
    VK_EXPECT_SUCCESS(vkAllocateMemory(device, &create_info, nullptr, &device_memory));
    return device_memory;
}

} // namespace

buffer::buffer(std::shared_ptr<device_context> context, size_t size,
    VkBufferUsageFlags usage, VkSharingMode sharing_mode) noexcept
    : buffer_(
          context, create_buffer(*context, size, usage, sharing_mode)),
     memory_(
          context, allocate_memory(*context, buffer_, size, usage, sharing_mode)) {}

} // namespace zoo::render::resources
