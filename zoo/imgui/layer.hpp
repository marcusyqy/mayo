#pragma once
#include "imgui.h"

#include "core/window.hpp"
#include "render/descriptor_pool.hpp"
#include "render/engine.hpp"

#include "render/resources/texture.hpp"

namespace zoo::imgui {

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

} // namespace zoo::imgui
