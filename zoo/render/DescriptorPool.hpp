#pragma once

#include "Pipeline.hpp"
#include "fwd.hpp"

namespace zoo::render {

class ResourceBindings {
public:
    ResourceBindings(VkDescriptorSet set) noexcept;
    ~ResourceBindings() noexcept = default;

    ResourceBindings() noexcept = default;

private:
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
