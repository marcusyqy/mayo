#include "RenderPass.hpp"

namespace zoo::render {

namespace {

VkRenderPass create_vk_renderpass(DeviceContext& context, VkFormat format, VkFormat depth) noexcept {
    VkAttachmentDescription color_attachment{};
    color_attachment.format         = format;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format         = depth;
    depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depth_dependency = {};
    depth_dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    depth_dependency.dstSubpass          = 0;
    depth_dependency.srcStageMask =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.srcAccessMask = 0;
    depth_dependency.dstStageMask =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    // is this really needed(?)
    std::array attachments  = { color_attachment, depth_attachment };
    std::array dependencies = { dependency, depth_dependency };

    VkRenderPassCreateInfo renderpass_info{};
    renderpass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_info.attachmentCount = uint32_t(attachments.size());
    renderpass_info.pAttachments    = attachments.data();
    renderpass_info.subpassCount    = 1;
    renderpass_info.pSubpasses      = &subpass;
    renderpass_info.dependencyCount = uint32_t(dependencies.size());
    renderpass_info.pDependencies   = dependencies.data();

    VkRenderPass renderpass{};
    VK_EXPECT_SUCCESS(vkCreateRenderPass(context, &renderpass_info, nullptr, &renderpass));

    return renderpass;
}

VkRenderPass create_renderpass(DeviceContext& context, stdx::span<AttachmentDescription> descriptions) noexcept {
    // initialize counts
    u32 attachment_count{}, subpass_count{}, dependency_count{};
    constexpr size_t ATTACHMENT_MAX_SIZE = 5;
    VkAttachmentDescription attachments[ATTACHMENT_MAX_SIZE]{};
    VkSubpassDescription subpasses[ATTACHMENT_MAX_SIZE]{};
    VkSubpassDependency dependencies[ATTACHMENT_MAX_SIZE]{};
    VkAttachmentReference references[ATTACHMENT_MAX_SIZE]{};

    // initialize the data with descriptions
    for (auto& description : descriptions) {
    }

    VkRenderPassCreateInfo renderpass_info{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachment_count,
        .pAttachments    = attachments,
        .subpassCount    = subpass_count,
        .pSubpasses      = +subpasses,
        .dependencyCount = dependency_count,
        .pDependencies   = +dependencies,
    };

    VkRenderPass renderpass{};
    VK_EXPECT_SUCCESS(vkCreateRenderPass(context, &renderpass_info, nullptr, &renderpass));
    return renderpass;
}

} // namespace

ColorAttachmentDescription::ColorAttachmentDescription(VkFormat format) noexcept :
    AttachmentDescription{ VkAttachmentDescription{
                               .format         = format,
                               .samples        = VK_SAMPLE_COUNT_1_BIT,
                               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                               .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                           },
                           VkSubpassDependency{
                               .srcSubpass    = VK_SUBPASS_EXTERNAL,
                               .dstSubpass    = 0,
                               .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               .srcAccessMask = 0,
                               .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                           },
                           VkImageLayout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                           Type::color } {}

DepthAttachmentDescription::DepthAttachmentDescription() noexcept :
    AttachmentDescription{
        VkAttachmentDescription{
            .format         = VK_FORMAT_D32_SFLOAT,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
        VkSubpassDependency{
            .srcSubpass    = VK_SUBPASS_EXTERNAL,
            .dstSubpass    = 0,
            .srcStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        VkImageLayout{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
        Type::depth
    } {}

RenderPass::RenderPass() noexcept : context_(nullptr), underlying_(nullptr) {}

RenderPass::RenderPass(DeviceContext& context, VkFormat format, VkFormat depth) noexcept :
    context_(&context), underlying_(create_vk_renderpass(*context_, format, depth)) {}

RenderPass::RenderPass(DeviceContext& context, stdx::span<AttachmentDescription> descriptions) noexcept :
    context_(&context), underlying_(create_vk_renderpass(*context_, descriptions)) {}

void RenderPass::reset() noexcept {
    if (context_ != nullptr) {
        context_->release_device_resource(release());
        context_ = nullptr;
    }
}

RenderPass& RenderPass::operator=(RenderPass&& renderpass) noexcept {
    reset();
    context_    = std::move(renderpass.context_);
    underlying_ = std::move(renderpass.underlying_);
    renderpass.release();
    return *this;
}

RenderPass::underlying_type RenderPass::release() noexcept {
    underlying_type ret = underlying_;
    underlying_         = nullptr;
    context_            = nullptr;
    return ret;
}

RenderPass::RenderPass(RenderPass&& renderpass) noexcept :
    context_(std::move(renderpass.context_)), underlying_(std::move(renderpass.underlying_)) {
    renderpass.release();
}

void RenderPass::emplace(DeviceContext& device, underlying_type type) noexcept {
    context_    = std::addressof(device);
    underlying_ = type;
}

RenderPass::~RenderPass() noexcept { reset(); }

} // namespace zoo::render
