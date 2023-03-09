#include "RenderPass.hpp"

namespace zoo::render {

namespace {
VkRenderPass create_vk_renderpass(
    DeviceContext& context, VkFormat format) noexcept {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_info.attachmentCount = 1;
    renderpass_info.pAttachments = &color_attachment;
    renderpass_info.subpassCount = 1;
    renderpass_info.pSubpasses = &subpass;
    renderpass_info.dependencyCount = 1;
    renderpass_info.pDependencies = &dependency;

    VkRenderPass renderpass{};
    VK_EXPECT_SUCCESS(
        vkCreateRenderPass(context, &renderpass_info, nullptr, &renderpass));

    return renderpass;
}

} // namespace

RenderPass::RenderPass() noexcept : context_(nullptr), underlying_(nullptr) {}

RenderPass::RenderPass(DeviceContext& context, VkFormat format) noexcept
    : context_(std::addressof(context)),
      underlying_(create_vk_renderpass(context, format)) {}

void RenderPass::reset() noexcept {
    if (context_ != nullptr) {
        context_->release_device_resource(release());
        context_ = nullptr;
    }
}

RenderPass& RenderPass::operator=(RenderPass&& renderpass) noexcept {
    reset();
    context_ = std::move(renderpass.context_);
    underlying_ = std::move(renderpass.underlying_);
    renderpass.release();
    return *this;
}

RenderPass::underlying_type RenderPass::release() noexcept {
    underlying_type ret = underlying_;
    underlying_ = nullptr;
    context_ = nullptr;
    return ret;
}

RenderPass::RenderPass(RenderPass&& renderpass) noexcept
    : context_(std::move(renderpass.context_)),
      underlying_(std::move(renderpass.underlying_)) {
    renderpass.release();
}

void RenderPass::emplace(DeviceContext& device, underlying_type type) noexcept {
    context_ = std::addressof(device);
    underlying_ = type;
}

RenderPass::~RenderPass() noexcept { reset(); }

} // namespace zoo::render
