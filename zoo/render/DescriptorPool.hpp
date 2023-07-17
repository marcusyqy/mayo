#pragma once

#include "Pipeline.hpp"
#include "fwd.hpp"
#include "resources/Buffer.hpp"

namespace zoo::render {

class ResourceBindings {
public:
    ResourceBindings(DeviceContext& context, VkDescriptorPool pool,
        VkDescriptorSet set) noexcept;

    ResourceBindings() noexcept = default;
    ~ResourceBindings() noexcept;

    struct BindingBatch {
        BindingBatch& bind(u32 binding, resources::Buffer& buffer) noexcept;
        void end_batch() noexcept;

        BindingBatch(ResourceBindings& target) : target_(target) {}

    private:
        static constexpr u32 MAX_RESOURCE_SIZE = 3;
        VkDescriptorBufferInfo buffer_infos_[MAX_RESOURCE_SIZE] = {};
        u32 buffer_count_ = 0;

        VkWriteDescriptorSet write_descriptors_[MAX_RESOURCE_SIZE] = {};
        u32 write_descriptors_count_ = 0;

    private:
        friend ResourceBindings;
        ResourceBindings& target_;
    };

    // should not need this since `context_` is a reference :D
    ResourceBindings(ResourceBindings&& o) noexcept;
    ResourceBindings& operator=(ResourceBindings&& o) noexcept;

    BindingBatch start_batch() noexcept;
    void write(const BindingBatch& batch) noexcept;
    void release() noexcept;

    operator bool() const noexcept { return valid(); }
    bool valid() const noexcept { return set_ != nullptr; }

    VkDescriptorSet set() const noexcept { return set_; }

private:
    void reset_members() noexcept;
    void release_allocation() noexcept;

private:
    DeviceContext* context_ = nullptr;
    VkDescriptorPool pool_ = nullptr;
    VkDescriptorSet set_ = nullptr;
};

class DescriptorPool {
public:
    // TODO: keep resource count.
    ResourceBindings allocate(render::Pipeline& pipeline) noexcept;

    DescriptorPool(const DescriptorPool&) = default;
    DescriptorPool(DescriptorPool&&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;
    DescriptorPool& operator=(DescriptorPool&&) = delete;

    // TODO: extend to have some sort of settings to contain all the possible
    // resource that we can have.
    DescriptorPool(DeviceContext& context) noexcept;
    ~DescriptorPool() noexcept;

private:
    DeviceContext& context_;
    VkDescriptorPool pool_;
};

} // namespace zoo::render
