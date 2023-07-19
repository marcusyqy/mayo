#include "DescriptorPool.hpp"

namespace zoo::render {

ResourceBindings::BindingBatch& ResourceBindings::BindingBatch::bind(
    u32 binding,
    VkBuffer buffer,
    u32 offset,
    u32 size,
    VkDescriptorType bind_type) noexcept {
    ZOO_ASSERT(buffer_count_ < MAX_RESOURCE_SIZE, "Need to increase number of max resource size or we have a bug");

    VkDescriptorBufferInfo& buffer_info = buffer_infos_[buffer_count_++];
    buffer_info.buffer                  = buffer;
    buffer_info.offset                  = offset;
    buffer_info.range                   = size;

    ZOO_ASSERT(
        write_descriptors_count_ < MAX_RESOURCE_SIZE,
        "Need to increase number of max resource size or we have a bug");

    VkWriteDescriptorSet& set_write = write_descriptors_[write_descriptors_count_++];

    set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set_write.pNext = nullptr;

    set_write.dstBinding = binding;
    set_write.dstSet     = target_.set();

    set_write.descriptorCount = 1;
    set_write.descriptorType  = bind_type;

    set_write.pBufferInfo = &buffer_info;

    return *this;
}

ResourceBindings::BindingBatch&
    ResourceBindings::BindingBatch::bind(u32 binding, resources::Buffer& buffer, VkDescriptorType bind_type) noexcept {
    bind(binding, buffer.handle(), 0, buffer.allocated_size(), bind_type);
    return *this;
}

ResourceBindings::BindingBatch& ResourceBindings::BindingBatch::bind(
    u32 binding,
    resources::BufferView& buffer,
    VkDescriptorType bind_type) noexcept {
    auto [start, end] = buffer.span();
    bind(binding, buffer.handle(), start, end - start, bind_type);
    return *this;
}

void ResourceBindings::BindingBatch::end_batch() noexcept { target_.write(*this); }

void ResourceBindings::write(const BindingBatch& batch) noexcept {
    if (context_ != nullptr) {
        vkUpdateDescriptorSets(*context_, batch.write_descriptors_count_, batch.write_descriptors_, 0, nullptr);
    }
}

ResourceBindings::BindingBatch ResourceBindings::start_batch() noexcept { return { *this }; }

ResourceBindings::ResourceBindings(DeviceContext& context, VkDescriptorPool pool, VkDescriptorSet set) noexcept :
    context_(&context), pool_(pool), set_(set) {}

ResourceBindings::~ResourceBindings() noexcept { release(); }

void ResourceBindings::release() noexcept {
    // so that release can be called multiple times.
    release_allocation();
    reset_members();
}

void ResourceBindings::reset_members() noexcept {
    context_ = nullptr;
    pool_    = nullptr;
    set_     = nullptr;
}

void ResourceBindings::release_allocation() noexcept {

    // BUG: Since we don't call
    // `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT` we cannot free. For
    // now we will simply take this overhead but it is very unnecessary.
    if (context_ != nullptr && pool_ != nullptr && set_ != nullptr) vkFreeDescriptorSets(*context_, pool_, 1, &set_);
}

ResourceBindings::ResourceBindings(ResourceBindings&& o) noexcept { *this = std::move(o); }
ResourceBindings& ResourceBindings::operator=(ResourceBindings&& o) noexcept {
    release_allocation();

    context_ = o.context_;
    pool_    = o.pool_;
    set_     = o.set_;

    o.reset_members();

    return *this;
}

DescriptorPool::~DescriptorPool() noexcept {
    if (pool_ != nullptr) vkDestroyDescriptorPool(context_, pool_, nullptr);
}

// TODO: create a default use case for descriptor pool
DescriptorPool::DescriptorPool(DeviceContext& context) noexcept : context_(context), pool_(nullptr) {
    constexpr u32 MAX_POOL_SIZE = 10;

    // clang-format off
    VkDescriptorPoolSize sizes[] {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_POOL_SIZE },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_POOL_SIZE }
    };
    // clang-format on

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        // TODO: we can remove this if needed. This is needed for solving the
        // bug above ^^
        .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets       = MAX_POOL_SIZE,
        .poolSizeCount = (u32)std::size(sizes),
        .pPoolSizes    = +sizes,
    };

    VK_EXPECT_SUCCESS(vkCreateDescriptorPool(context_, &pool_info, nullptr, &pool_));
}

ResourceBindings DescriptorPool::allocate(render::Pipeline& pipeline) noexcept {

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = pool_,
        .descriptorSetCount = 1,
        .pSetLayouts        = &pipeline.set_layout_,
    };

    VkDescriptorSet descriptor = nullptr;
    VK_EXPECT_SUCCESS(vkAllocateDescriptorSets(context_, &alloc_info, &descriptor));

    return { context_, pool_, descriptor };
} // namespace zoo::render

} // namespace zoo::render
