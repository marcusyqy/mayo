#pragma once

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
    Imgui_Scene(render::Engine& engine, s32 width, s32 height) noexcept;
    ~Imgui_Scene() noexcept;

    void init() noexcept;
    void allocate_frame_buffer(const render::Pipeline& pipeline) noexcept;
    void exit() noexcept;
    const render::Resource_Bindings*
        ensure_frame_buffers_and_update(const render::Pipeline& pipeline, s32 width, s32 height) noexcept;

private:
    static constexpr s32 MAX_FRAMES  = 3;
    static constexpr s32 MAX_OBJECTS = 10'000;

    const render::Resource_Bindings& update() noexcept;

private:
    render::Engine& engine_;
    s32 width_;
    s32 height_;

    render::Pipeline pipeline_;
    render::Render_Pass renderpass_;
    render::Descriptor_Pool descriptor_pool_;
    render::resources::Buffer scene_data_buffer_;

    render::resources::Mesh mesh_;
    render::resources::Texture lost_empire_;
    render::resources::TextureSampler lost_empire_sampler_;

    struct Frame_Data {
        render::resources::Buffer uniform_buffer;
        render::Resource_Bindings bindings;
        render::resources::Buffer object_storage_buffer;
        render::resources::TextureSampler render_sampler;

        // sync stuff
        render::scene::Command_Buffer command_buffer;
        render::sync::Fence in_flight_fence;

        // resize stuff
        render::Resource_Bindings render_binding;
        render::resources::Texture render_buffer;
        render::resources::Texture depth_buffer;
        render::Framebuffer render_target;

        s32 width;
        s32 height;
    };

    s32 index_ = 0;
    Frame_Data frame_datas_[MAX_FRAMES];
    std::chrono::high_resolution_clock::time_point start_time_;
};
} // namespace zoo
