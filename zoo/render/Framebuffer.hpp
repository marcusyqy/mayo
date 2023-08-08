
#pragma once
#include "RenderPass.hpp"
#include "resources/Texture.hpp"
#include <stdx/span.hpp>

namespace zoo::render {

class Framebuffer {
public:
    Framebuffer()                              = default;
    Framebuffer(const Framebuffer&)            = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer& operator=(Framebuffer&&) noexcept;
    Framebuffer(Framebuffer&&) noexcept;

    Framebuffer(
        render::DeviceContext& context,
        const RenderPass& renderpass,
        stdx::span<const resources::TextureView*> render_targets,
        u32 width,
        u32 height,
        u32 layers = 1) noexcept;

    ~Framebuffer() noexcept;

    operator VkFramebuffer() const noexcept { return get(); }
    VkFramebuffer get() const noexcept { return underlying_; }

    VkRenderPass renderpass() const noexcept { return renderpass_; }
    u32 width() const noexcept { return width_; }
    u32 height() const noexcept { return height_; }

private:
    DeviceContext* context_   = nullptr;
    VkFramebuffer underlying_ = nullptr;
    VkRenderPass renderpass_  = nullptr;

    u32 width_  = {};
    u32 height_ = {};
    u32 layers_ = {};
};

} // namespace zoo::render
