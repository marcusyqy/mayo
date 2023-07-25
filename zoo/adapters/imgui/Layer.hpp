#pragma once
#include "imgui.h"

#include "render/Engine.hpp"

namespace zoo::adapters::imgui {

struct DrawData {};

class Layer {
public:
    void init() noexcept;
    void exit() noexcept;

    void update() noexcept;
    void render() noexcept;

    Layer(render::Engine& engine) noexcept;
    ~Layer() noexcept;

private:
    render::Engine* engine_;
};

} // namespace zoo::adapters::imgui
