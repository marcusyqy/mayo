#pragma once
#include "imgui.h"

#include "core/Window.hpp"
#include "render/Descriptor_Pool.hpp"
#include "render/Engine.hpp"

#include "render/resources/Texture.hpp"

namespace zoo::adapters::imgui {

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
};

} // namespace zoo::adapters::imgui
