#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "render/pipeline.hpp"
#include "render/renderpass.hpp"
#include "stdx/function_ref.hpp"

namespace zoo::render::scene {

class command_buffer {
public:
    using underlying_type = VkCommandBuffer;

    underlying_type release() noexcept;
    void reset() noexcept;
    void clear() noexcept;

    command_buffer(std::shared_ptr<device_context> context) noexcept;
    command_buffer(command_buffer&& other) noexcept;
    command_buffer& operator=(command_buffer&& other) noexcept;
    ~command_buffer() noexcept;

    void set_viewport(const VkViewport& viewport) noexcept;
    void set_scissor(const VkRect2D& scissor) noexcept;

    // bindings
    void bind(const render::pipeline& pipeline) noexcept;

    // maybe struct?
    void draw(uint32_t vertex_count, uint32_t instance_count,
        uint32_t first_vertex, uint32_t first_instance) noexcept;

    void exec(const VkRenderPassBeginInfo& begin_info,
        stdx::function_ref<void()> c) noexcept;
    void record(stdx::function_ref<void()> c) noexcept;

    void submit(operation op_type, stdx::span<VkSemaphore> wait_semaphores,
        stdx::span<VkSemaphore> signal_semaphores) noexcept;

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
