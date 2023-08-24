#pragma once

#include "pipeline.hpp"
#include "fwd.hpp"
#include "resources/buffer.hpp"
#include "resources/texture.hpp"

namespace zoo::render {

// Forward Declare.
class Resource_Bindings;

struct Binding_Batch {

    Binding_Batch& bind(u32 binding, resources::Buffer& buffer, VkDescriptorType bind_type) noexcept;
    Binding_Batch& bind(u32 binding, resources::BufferView& buffer, VkDescriptorType bind_type) noexcept;
    Binding_Batch& bind(u32 set, u32 binding, resources::Buffer& buffer, VkDescriptorType bind_type) noexcept;
    Binding_Batch& bind(u32 set, u32 binding, resources::BufferView& buffer, VkDescriptorType bind_type) noexcept;

    Binding_Batch&
        bind(u32 set, u32 binding, VkBuffer buffer, u32 offset, u32 size, VkDescriptorType bind_type) noexcept;

    Binding_Batch& bind(
        u32 binding,
        resources::Texture& texture,
        resources::TextureSampler& sampler,
        VkDescriptorType bind_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) noexcept;

    Binding_Batch& bind(
        u32 set,
        u32 binding,
        resources::Texture& texture,
        resources::TextureSampler& sampler,
        VkDescriptorType bind_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) noexcept;

    void end_batch() noexcept;

    Binding_Batch(Resource_Bindings& target) noexcept : target_(target) {}

private:
    static constexpr u32 MAX_RESOURCE_SIZE                  = 5;
    VkDescriptorBufferInfo buffer_infos_[MAX_RESOURCE_SIZE] = {};
    u32 buffer_count_                                       = 0;

    VkDescriptorImageInfo texture_infos_[MAX_RESOURCE_SIZE] = {};
    u32 texture_count_                                      = 0;

    VkWriteDescriptorSet write_descriptors_[MAX_RESOURCE_SIZE] = {};
    u32 write_descriptors_count_                               = 0;

private:
    friend class Resource_Bindings;
    Resource_Bindings& target_;
};

class Resource_Bindings {
public:
    static constexpr u32 MAX_RESOURCE_SIZE = Binding_Batch::MAX_RESOURCE_SIZE;
    Resource_Bindings(Device_Context& context, VkDescriptorPool pool, stdx::span<VkDescriptorSet> sets) noexcept;

    Resource_Bindings() noexcept = default;
    ~Resource_Bindings() noexcept;

    // should not need this since `context_` is a reference :D
    Resource_Bindings(Resource_Bindings&& o) noexcept;
    Resource_Bindings& operator=(Resource_Bindings&& o) noexcept;

    Binding_Batch start_batch() noexcept;
    void write(const Binding_Batch& batch) noexcept;
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
    Device_Context* context_                = nullptr;
    VkDescriptorPool pool_                  = nullptr;
    VkDescriptorSet set_[MAX_RESOURCE_SIZE] = {};
    u32 set_count_                          = {};
};

class Descriptor_Pool {
public:
    // TODO: keep resource count.
    Resource_Bindings allocate(render::Pipeline& pipeline) noexcept;

    Descriptor_Pool(const Descriptor_Pool&) noexcept            = delete;
    Descriptor_Pool& operator=(const Descriptor_Pool&) noexcept = delete;

    Descriptor_Pool(Descriptor_Pool&&) noexcept;
    Descriptor_Pool& operator=(Descriptor_Pool&&) noexcept;

    // TODO: extend to have some sort of settings to contain all the possible
    // resource that we can have.
    static constexpr u32 DEFAULT_MAX_POOL_SIZE = 10;
    Descriptor_Pool(Device_Context& context, u32 pool_size = DEFAULT_MAX_POOL_SIZE) noexcept;
    Descriptor_Pool() noexcept = default;
    ~Descriptor_Pool() noexcept;

    inline operator bool() const noexcept { return valid(); }
    inline bool valid() const noexcept { return pool_ != nullptr; }

private:
    Device_Context* context_ = nullptr;
    VkDescriptorPool pool_   = nullptr;
};

} // namespace zoo::render
