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
    VkDescriptorSet set_ = nullptr;
};

class DescriptorPool {
public:
    // TODO: keep resource count.
    ResourceBindings allocate(render::Pipeline& pipeline) noexcept;

    // TODO: extend to have some sort of settings to contain all the possible
    // resource that we can have.
    DescriptorPool(DeviceContext& context) noexcept;

private:
    DeviceContext& context_;
    VkDescriptorPool pool_;
};

} // namespace zoo::render
