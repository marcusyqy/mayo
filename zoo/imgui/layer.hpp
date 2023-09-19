#pragma once
#include "imgui.h"

#include "core/window.hpp"
#include "render/descriptor_pool.hpp"
#include "render/engine.hpp"

#include "render/resources/texture.hpp"
#include "imgui/scene.hpp"

namespace zoo::imgui {

class Layer {
public:
    void update() noexcept;
    void render() noexcept;
    void resize(s32 x, s32 y) noexcept;

    Layer(const Layer&)            = delete;
    Layer(Layer&&)                 = delete;
    Layer& operator=(const Layer&) = delete;
    Layer& operator=(Layer&&)      = delete;

    Layer(render::Engine& engine, Window& window) noexcept;
    ~Layer() noexcept;

private:
    void init() noexcept;
    void exit() noexcept;

    void draw_frame_buffer() noexcept;

private:
    render::Engine& engine_;
    Window& window_;

    Imgui_Scene scene_;
};

} // namespace zoo::imgui
