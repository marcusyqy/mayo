#pragma once

#include "DeviceContext.hpp"
#include "fwd.hpp"

namespace zoo::render {

class RenderPass {
public:
    using underlying_type = VkRenderPass;

    void emplace(DeviceContext& device, underlying_type type) noexcept;
    void reset() noexcept;
    underlying_type release() noexcept;

    underlying_type get() const noexcept { return underlying_; }
    operator underlying_type() const noexcept { return get(); }

    RenderPass() noexcept;
    RenderPass(DeviceContext& context, VkFormat format, VkFormat depth) noexcept;

    ~RenderPass() noexcept;

    RenderPass& operator=(RenderPass&& renderpass) noexcept;
    RenderPass(RenderPass&& renderpass) noexcept;

    RenderPass& operator=(const RenderPass& renderpass) noexcept = delete;
    RenderPass(const RenderPass& renderpass) noexcept = delete;

private:
    DeviceContext* context_;
    underlying_type underlying_;
};

} // namespace zoo::render
