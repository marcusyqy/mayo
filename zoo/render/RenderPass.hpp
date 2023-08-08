#pragma once

#include "DeviceContext.hpp"
#include "fwd.hpp"
#include "stdx/span.hpp"

namespace zoo::render {

struct AttachmentDescription {
    VkAttachmentDescription description{
        .format         = VK_FORMAT_UNDEFINED,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkSubpassDependency dependency{
        .srcSubpass    = VK_SUBPASS_EXTERNAL,
        .dstSubpass    = 0,
        .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    // @NOTE: This are based on what `VkSubpassDescription` has in their body
    enum class Type { color, input, depth, resolve } type;
};

// These are helper functions to help with the creation of things that will be used often.
// @FIXME: should this be a struct or just a typical function?
// how would the api be used and what should be the correct representation of it?
struct ColorAttachmentDescription : AttachmentDescription {
    ColorAttachmentDescription(VkFormat format) noexcept;
};

// @FIXME: read comment above about `ColorAttachmentDescription`
struct DepthAttachmentDescription : AttachmentDescription {
    DepthAttachmentDescription() noexcept;
};

// @TODO: Make Renderpass reusable so that it doesn't need to live inside Swapchain.
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
    RenderPass(DeviceContext& context, stdx::span<AttachmentDescription> descriptions) noexcept;

    ~RenderPass() noexcept;

    RenderPass& operator=(RenderPass&& renderpass) noexcept;
    RenderPass(RenderPass&& renderpass) noexcept;

    RenderPass& operator=(const RenderPass& renderpass) noexcept = delete;
    RenderPass(const RenderPass& renderpass) noexcept            = delete;

private:
    DeviceContext* context_;
    underlying_type underlying_;
};

} // namespace zoo::render
