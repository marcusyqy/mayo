#include "renderpass.hpp"

namespace zoo::render {
namespace {
VkRenderPass create_vk_renderpass(
    std::shared_ptr<device_context> context, VkFormat format) noexcept {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

    VkRenderPass renderpass{};
    VK_EXPECT_SUCCESS(
        vkCreateRenderPass(*context, &renderpass_info, nullptr, &renderpass));

    return renderpass;
}

} // namespace

renderpass::renderpass(
    std::shared_ptr<device_context> context, VkFormat format) noexcept :
    underlying_type(context, create_vk_renderpass(context, format)) {}

renderpass& renderpass::operator=(renderpass&& renderpass) noexcept {
    underlying_type::reset();
    underlying_type::operator=(std::move(renderpass));
    std::ignore = renderpass.release();
    return *this;
}

renderpass::renderpass(renderpass&& renderpass) noexcept :
    underlying_type(std::move(renderpass)) {
    std::ignore = renderpass.release();
}

} // namespace zoo::render
