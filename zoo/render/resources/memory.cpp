#include "memory.hpp"

namespace zoo::render::resources {

namespace {
VkDeviceMemory allocate_memory(VkDevice device, size_t size) noexcept {
    VkDeviceBufferMemoryRequirements requirements{};
    vkGetDeviceBufferMemoryRequirements(
        device, &device_requirements_info, &memory_requirements);

    VkMemoryAllocateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    VkDeviceMemory device_memory{};
    VK_EXPECT_SUCCESS(
        vkAllocateMemory(device, &create_info, nullptr, &device_memory));
    return device_memory;
}
} // namespace
  //

memory(std::shared_ptr<device_context> context)
    : underlying_type(context_, allocate_memory(*context, size)) {}

} // namespace zoo::render::resources
