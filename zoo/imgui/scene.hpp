#pragma once

#if 0
#include "render/descriptor_pool.hpp"
#include "render/engine.hpp"
#include "render/framebuffer.hpp"
#include "render/pipeline.hpp"
#include "render/resources/buffer.hpp"
#include "render/resources/texture.hpp"
#include "render/scene/command_buffer.hpp"
#include "render/sync/fence.hpp"

namespace zoo {

struct Imgui_Scene {
    Imgui_Scene(render::Engine& engine) noexcept;

    void init() noexcept;
    void exit() noexcept;
    void update() noexcept;

    void set_target_image(s32 idx) noexcept;
    void resize(s32 x, s32 y) noexcept;

private:
    static constexpr s32 MAX_FRAMES  = 3;
    static constexpr s32 MAX_OBJECTS = 10'000;

private:
    render::Engine& engine_;

    render::Pipeline pipeline;
    render::Render_Pass renderpass;

    struct Frame_Data {
        render::resources::Buffer uniform_buffer;
        render::Resource_Bindings bindings;
        render::resources::Buffer object_storage_buffer;

        render::resources::Texture render_buffer;
        render::resources::Texture depth_buffer;
        render::Framebuffer render_target;

        render::scene::Command_Buffer command_buffer;
        render::sync::Fence in_flight_fence;
    };

    s32 index = -1;
    Frame_Data frame_data[MAX_FRAMES];

};
} // namespace zoo
#endif
