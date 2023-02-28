#include "Buffer.hpp"

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

} // namespace

Buffer::Buffer(DeviceContext& context, size_t size,
    VkBufferUsageFlags usage, VkSharingMode sharing_mode) noexcept
    : underlying_type(
          context, create_buffer(context, size, usage, sharing_mode)) {}

} // namespace zoo::render::resources
