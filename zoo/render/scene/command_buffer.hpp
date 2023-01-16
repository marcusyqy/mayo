#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "render/pipeline.hpp"
#include "render/renderpass.hpp"

namespace zoo::render::scene {

class command_buffer {
public:
    using underlying_type = VkCommandBuffer;

    underlying_type release() noexcept;
    void reset() noexcept;

    command_buffer(std::shared_ptr<device_context> context) noexcept;
    command_buffer(command_buffer&& other) noexcept;
    command_buffer& operator=(command_buffer&& other) noexcept;
    ~command_buffer() noexcept;

    void submit() noexcept;

    void set_viewport(const VkViewport& viewport) noexcept;
    void set_scissor(const VkRect2D& scissor) noexcept;

    // bindings
    void bind(const render::pipeline& pipeline) noexcept;

    // maybe struct?
    void draw(uint32_t vertex_count, uint32_t instance_count,
        uint32_t first_vertex, uint32_t first_instance) noexcept;

    template<typename Call>
    void exec(const VkRenderPassBeginInfo& begin_info, Call c) noexcept {
        begin_renderpass(begin_info);
        c();
        end_renderpass();
    }

    template<typename Call>
    void record(Call&& c) noexcept {
        start_record();
        c();
        end_record();
    }

private:
    void start_record() noexcept;
    void end_record() noexcept;

    void begin_renderpass(const VkRenderPassBeginInfo& begin_info) noexcept;
    void end_renderpass() noexcept;

private:
    std::shared_ptr<device_context> context_;
    underlying_type underlying_;
};
} // namespace zoo::render::scene
