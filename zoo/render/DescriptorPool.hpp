#pragma once

#include "Pipeline.hpp"
#include "fwd.hpp"
#include "resources/Buffer.hpp"
#include "resources/Texture.hpp"

namespace zoo::render {

// Forward Declare.
class ResourceBindings;

struct BindingBatch {

    BindingBatch& bind(u32 binding, resources::Buffer& buffer, VkDescriptorType bind_type) noexcept;
    BindingBatch& bind(u32 binding, resources::BufferView& buffer, VkDescriptorType bind_type) noexcept;
    BindingBatch& bind(u32 set, u32 binding, resources::Buffer& buffer, VkDescriptorType bind_type) noexcept;
    BindingBatch& bind(u32 set, u32 binding, resources::BufferView& buffer, VkDescriptorType bind_type) noexcept;

    BindingBatch&
        bind(u32 set, u32 binding, VkBuffer buffer, u32 offset, u32 size, VkDescriptorType bind_type) noexcept;

    BindingBatch& bind(
        u32 set,
        u32 binding,
        resources::Texture& texture,
        resources::TextureSampler& sampler,
        VkDescriptorType bind_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) noexcept;

    void end_batch() noexcept;

    BindingBatch(ResourceBindings& target) noexcept : target_(target) {}

private:
    static constexpr u32 MAX_RESOURCE_SIZE                  = 5;
    VkDescriptorBufferInfo buffer_infos_[MAX_RESOURCE_SIZE] = {};
    u32 buffer_count_                                       = 0;

    VkDescriptorImageInfo texture_infos_[MAX_RESOURCE_SIZE] = {};
    u32 texture_count_                                      = 0;

    VkWriteDescriptorSet write_descriptors_[MAX_RESOURCE_SIZE] = {};
    u32 write_descriptors_count_                               = 0;

private:
    friend class ResourceBindings;
    ResourceBindings& target_;
};

class ResourceBindings {
public:
    static constexpr u32 MAX_RESOURCE_SIZE = BindingBatch::MAX_RESOURCE_SIZE;
    ResourceBindings(DeviceContext& context, VkDescriptorPool pool, stdx::span<VkDescriptorSet> sets) noexcept;

    ResourceBindings() noexcept = default;
    ~ResourceBindings() noexcept;

    // should not need this since `context_` is a reference :D
    ResourceBindings(ResourceBindings&& o) noexcept;
    ResourceBindings& operator=(ResourceBindings&& o) noexcept;

    BindingBatch start_batch() noexcept;
    void write(const BindingBatch& batch) noexcept;
    void release() noexcept;

    operator bool() const noexcept { return valid(); }
    bool valid() const noexcept { return set_count_ != 0; }
    VkDescriptorSet set(u32 index) const noexcept { return set_[index]; }
    const VkDescriptorSet* sets() const noexcept { return set_; }
    u32 count() const noexcept { return set_count_; }

private:
    void reset_members() noexcept;
    void release_allocation() noexcept;

private:
    DeviceContext* context_                 = nullptr;
    VkDescriptorPool pool_                  = nullptr;
    VkDescriptorSet set_[MAX_RESOURCE_SIZE] = {};
    u32 set_count_                          = {};
};

class DescriptorPool {
public:
    // TODO: keep resource count.
    ResourceBindings allocate(render::Pipeline& pipeline) noexcept;

    DescriptorPool(const DescriptorPool&) noexcept            = delete;
    DescriptorPool& operator=(const DescriptorPool&) noexcept = delete;

    DescriptorPool(DescriptorPool&&) noexcept;
    DescriptorPool& operator=(DescriptorPool&&) noexcept;

    // TODO: extend to have some sort of settings to contain all the possible
    // resource that we can have.
    static constexpr u32 DEFAULT_MAX_POOL_SIZE = 10;
    DescriptorPool(DeviceContext& context, u32 pool_size = DEFAULT_MAX_POOL_SIZE) noexcept;
    DescriptorPool() noexcept = default;
    ~DescriptorPool() noexcept;

    inline operator bool() const noexcept { return valid(); }
    inline bool valid() const noexcept { return pool_ != nullptr; }

private:
    DeviceContext* context_ = nullptr;
    VkDescriptorPool pool_  = nullptr;
};

} // namespace zoo::render
