#include "DescriptorPool.hpp"

namespace zoo::render {

BindingBatch& BindingBatch::bind(
    u32 set,
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
    set_write.dstSet     = target_.set(set);

    set_write.descriptorCount = 1;
    set_write.descriptorType  = bind_type;

    set_write.pBufferInfo = &buffer_info;

    return *this;
}

BindingBatch& BindingBatch::bind(u32 binding, resources::Buffer& buffer, VkDescriptorType bind_type) noexcept {
    return bind(0, binding, buffer.handle(), 0, static_cast<u32>(buffer.allocated_size()), bind_type);
}

BindingBatch& BindingBatch::bind(u32 binding, resources::BufferView& buffer, VkDescriptorType bind_type) noexcept {
    auto [start, end] = buffer.span();
    return bind(0, binding, buffer.handle(), static_cast<u32>(start), static_cast<u32>(end - start), bind_type);
}

BindingBatch& BindingBatch::bind(u32 set, u32 binding, resources::Buffer& buffer, VkDescriptorType bind_type) noexcept {
    return bind(set, binding, buffer.handle(), 0, static_cast<u32>(buffer.allocated_size()), bind_type);
}

BindingBatch&
    BindingBatch::bind(u32 set, u32 binding, resources::BufferView& buffer, VkDescriptorType bind_type) noexcept {
    auto [start, end] = buffer.span();
    return bind(set, binding, buffer.handle(), static_cast<u32>(start), static_cast<u32>(end - start), bind_type);
}
BindingBatch& BindingBatch::bind(
    u32 binding,
    resources::Texture& texture,
    resources::TextureSampler& sampler,
    VkDescriptorType bind_type /*= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER */) noexcept {
    return bind(0, binding, texture, sampler, bind_type);
}

BindingBatch& BindingBatch::bind(
    u32 set,
    u32 binding,
    resources::Texture& texture,
    resources::TextureSampler& sampler,
    VkDescriptorType bind_type /*= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER */) noexcept {
    ZOO_ASSERT(texture_count_ < MAX_RESOURCE_SIZE, "Need to increase number of max resource size or we have a bug");

    VkDescriptorImageInfo& image_info = texture_infos_[texture_count_++];
    image_info.sampler                = sampler;
    image_info.imageView              = texture.view();
    image_info.imageLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ZOO_ASSERT(
        write_descriptors_count_ < MAX_RESOURCE_SIZE,
        "Need to increase number of max resource size or we have a bug");

    VkWriteDescriptorSet& set_write = write_descriptors_[write_descriptors_count_++];

    set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set_write.pNext = nullptr;

    set_write.dstBinding = binding;
    set_write.dstSet     = target_.set(set);

    set_write.descriptorCount = 1;
    set_write.descriptorType  = bind_type;

    set_write.pImageInfo = &image_info;

    return *this;
}

void BindingBatch::end_batch() noexcept { target_.write(*this); }

void ResourceBindings::write(const BindingBatch& batch) noexcept {
    if (context_ != nullptr) {
        vkUpdateDescriptorSets(*context_, batch.write_descriptors_count_, batch.write_descriptors_, 0, nullptr);
    }
}

BindingBatch ResourceBindings::start_batch() noexcept { return { *this }; }

ResourceBindings::ResourceBindings(
    DeviceContext& context,
    VkDescriptorPool pool,
    stdx::span<VkDescriptorSet> set) noexcept :
    context_(&context),
    pool_(pool), set_count_(static_cast<u32>(set.size())) {

    ZOO_ASSERT(set_count_ < MAX_RESOURCE_SIZE, "Must be within the specified size expected");
    std::copy(set.begin(), set.end(), std::begin(set_));
}

ResourceBindings::~ResourceBindings() noexcept { release(); }

void ResourceBindings::release() noexcept {
    // so that release can be called multiple times.
    release_allocation();
    reset_members();
}

void ResourceBindings::reset_members() noexcept {
    context_ = nullptr;
    pool_    = nullptr;

    for (u32 i = 0; i < set_count_; ++i) {
        set_[i] = nullptr;
    }
    set_count_ = 0;
}

void ResourceBindings::release_allocation() noexcept {

    // BUG: Since we don't call
    // `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT` we cannot free. For
    // now we will simply take this overhead but it is very unnecessary.
    if (context_ != nullptr && pool_ != nullptr) {
        for (u32 i = 0; i < set_count_; ++i) {
            vkFreeDescriptorSets(*context_, pool_, 1, &set_[i]);
        }
    }
}

ResourceBindings::ResourceBindings(ResourceBindings&& o) noexcept { *this = std::move(o); }

ResourceBindings& ResourceBindings::operator=(ResourceBindings&& o) noexcept {
    release_allocation();

    context_ = o.context_;
    pool_    = o.pool_;
    std::copy(o.set_, o.set_ + o.set_count_, set_);
    set_count_ = o.set_count_;
    o.reset_members();

    return *this;
}

DescriptorPool::~DescriptorPool() noexcept {
    if (pool_ != nullptr) vkDestroyDescriptorPool(*context_, pool_, nullptr);
}

DescriptorPool::DescriptorPool(DescriptorPool&& o) noexcept { *this = std::move(o); }

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& o) noexcept {
    // destroy
    this->~DescriptorPool();

    context_ = o.context_;
    pool_    = o.pool_;

    o.context_ = nullptr;
    o.pool_    = nullptr;
    return *this;
}

// TODO: create a default use case for descriptor pool
DescriptorPool::DescriptorPool(DeviceContext& context, u32 pool_size) noexcept : context_(&context), pool_(nullptr) {

    // clang-format off
    VkDescriptorPoolSize sizes[] {
        { VK_DESCRIPTOR_TYPE_SAMPLER, pool_size },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pool_size },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, pool_size },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pool_size },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, pool_size },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, pool_size },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pool_size }
    };
    // clang-format on

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        // TODO: we can remove this if needed. This is needed for solving the bug above.
        .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets       = pool_size * static_cast<u32>(std::size(sizes)),
        .poolSizeCount = static_cast<u32>(std::size(sizes)),
        .pPoolSizes    = +sizes,
    };

    VK_EXPECT_SUCCESS(vkCreateDescriptorPool(*context_, &pool_info, nullptr, &pool_));
}

ResourceBindings DescriptorPool::allocate(render::Pipeline& pipeline) noexcept {
    static_assert(
        ResourceBindings::MAX_RESOURCE_SIZE == render::Pipeline::MAX_DESCRIPTORS,
        "Must match the descriptors for the arrays");

    ZOO_ASSERT(valid(), "Must be well defined!");

    VkDescriptorSet descriptor[ResourceBindings::MAX_RESOURCE_SIZE] = {};
    u32 count                                                       = 0;

    for (; count < pipeline.set_layout_count_; ++count) {

        VkDescriptorSetAllocateInfo alloc_info = {
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext              = nullptr,
            .descriptorPool     = pool_,
            .descriptorSetCount = 1,
            .pSetLayouts        = &pipeline.set_layout_[count],
        };
        VK_EXPECT_SUCCESS(vkAllocateDescriptorSets(*context_, &alloc_info, &descriptor[count]));
    }

    return { *context_, pool_, { descriptor, count } };
}

} // namespace zoo::render
