
#pragma once
#include "RenderPass.hpp"
#include "resources/Texture.hpp"
#include <stdx/span.hpp>

namespace zoo::render {

class Framebuffer {
public:
    Framebuffer(const RenderPass& renderpass, stdx::span<resources::TextureView> render_targets) noexcept;
    ~Framebuffer() noexcept;

private:
};

} // namespace zoo::render
