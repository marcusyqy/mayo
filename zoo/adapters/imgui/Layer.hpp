#pragma once
#include "imgui.h"

#include "core/platform/Window.hpp"
#include "render/DescriptorPool.hpp"
#include "render/Engine.hpp"

#include "render/resources/Texture.hpp"

namespace zoo::adapters::imgui {

struct DrawData {
    render::resources::Texture font_texture;
    render::resources::TextureSampler font_sampler;
    render::Pipeline pipeline;
    render::DescriptorPool descriptor_pool;
};

struct FrameData {
    render::DescriptorPool descriptor_pool;
};

class Layer {
public:
    void init() noexcept;
    void exit() noexcept;

    void update() noexcept;
    void render() noexcept;

    Layer(render::Engine& engine, Window& window) noexcept;
    ~Layer() noexcept;

private:
    render::Engine* engine_;
    Window* window_;

    DrawData draw_data_;
    FrameData frame_data_[3];
};

} // namespace zoo::adapters::imgui
