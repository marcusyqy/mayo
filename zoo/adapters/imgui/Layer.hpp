#pragma once
#include "imgui.h"

#include "core/platform/Window.hpp"
#include "render/DescriptorPool.hpp"
#include "render/Engine.hpp"

#include "render/resources/Texture.hpp"

#include "main/Info.hpp"

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

application::ExitStatus test();

} // namespace zoo::adapters::imgui