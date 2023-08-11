#include "Framebuffer.hpp"

namespace zoo::render {

namespace {
VkFramebuffer create_framebuffer(
    render::DeviceContext& context,
    const RenderPass& renderpass,
    stdx::span<const resources::TextureView*> render_targets,
    u32 width,
    u32 height,
    u32 layers) noexcept {

    constexpr u32 MAX_ATTACHMENTS = 5;
    VkImageView attachments[MAX_ATTACHMENTS];
    u32 attachments_count = {};

    // @TODO: we should do some checking of formats with renderpass here.
    ZOO_ASSERT(
        (u32)render_targets.size() <= MAX_ATTACHMENTS,
        "Cannot have more render targets than `MAX_ATTACHMENTS`!");
    for (const auto& tv : render_targets) {
        ZOO_ASSERT(tv != nullptr);
        attachments[attachments_count++] = *tv;
    }

    VkFramebufferCreateInfo framebuffer_create_info{};
    framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass      = renderpass;
    framebuffer_create_info.attachmentCount = attachments_count;
    framebuffer_create_info.pAttachments    = +attachments;
    framebuffer_create_info.width           = width;
    framebuffer_create_info.height          = height;
    framebuffer_create_info.layers          = layers;

    VkFramebuffer framebuffer{};
    VK_EXPECT_SUCCESS(vkCreateFramebuffer(context, &framebuffer_create_info, nullptr, &framebuffer));
    return framebuffer;
}
} // namespace

Framebuffer& Framebuffer::operator=(Framebuffer&& o) noexcept {
    // call destructor
    this->~Framebuffer();

    context_    = o.context_;
    underlying_ = o.underlying_;
    renderpass_ = o.renderpass_;
    width_      = o.width_;
    height_     = o.height_;
    layers_     = o.layers_;

    o.context_    = nullptr;
    o.underlying_ = nullptr;
    o.renderpass_ = nullptr;
    o.width_      = 0;
    o.height_     = 0;
    o.layers_     = 0;

    return *this;
}

Framebuffer::Framebuffer(Framebuffer&& o) noexcept { *this = std::move(o); }

Framebuffer::Framebuffer(
    render::DeviceContext& context,
    const RenderPass& renderpass,
    stdx::span<const resources::TextureView*> render_targets,
    u32 width,
    u32 height,
    u32 layers) noexcept :
    context_(&context),
    underlying_(create_framebuffer(*context_, renderpass, render_targets, width, height, layers)),
    renderpass_(renderpass), width_(width), height_(height), layers_(layers) {}

Framebuffer::~Framebuffer() noexcept {
    if (context_ != nullptr && underlying_ != nullptr) vkDestroyFramebuffer(*context_, underlying_, nullptr);
}

} // namespace zoo::render
