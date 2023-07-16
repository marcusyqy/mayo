#include "DescriptorPool.hpp"

namespace zoo::render {

ResourceBindings::ResourceBindings(VkDescriptorSet set) noexcept : set_(set) {}

DescriptorPool::~DescriptorPool() noexcept
{
    if(pool_ != nullptr)
        vkDestroyDescriptorPool(context_, pool_, nullptr);
}

DescriptorPool::DescriptorPool(DeviceContext& context) noexcept
    : context_(context), pool_(nullptr) {

    // clang-format off
    VkDescriptorPoolSize sizes[] {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
    };
    // clang-format on

    VkDescriptorPoolCreateInfo pool_info = {
        .maxSets = 10,
        .poolSizeCount = (uint32_t)std::size(sizes),
        .pPoolSizes = +sizes,
    };

    VK_EXPECT_SUCCESS(
        vkCreateDescriptorPool(context_, &pool_info, nullptr, &pool_));
} // namespace zoo::render

ResourceBindings DescriptorPool::allocate(render::Pipeline& pipeline) noexcept {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool_,
        .descriptorSetCount = 1,
        .pSetLayouts = &pipeline.set_layout_,
    };

    VkDescriptorSet descriptor = nullptr;
    VK_EXPECT_SUCCESS(
        vkAllocateDescriptorSets(context_, &alloc_info, &descriptor));

    return {};
}
} // namespace zoo::render
