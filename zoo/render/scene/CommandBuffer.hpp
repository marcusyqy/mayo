#pragma once
#include "render/DeviceContext.hpp"
#include "render/Pipeline.hpp"
#include "render/Renderpass.hpp"
#include "render/fwd.hpp"
#include "stdx/function_ref.hpp"
#include "zoo.hpp"

namespace zoo::render::scene {

class CommandBuffer {
public:
    using underlying_type = VkCommandBuffer;

    underlying_type release() noexcept;
    void reset() noexcept;
    void clear() noexcept;

    CommandBuffer(DeviceContext& context) noexcept;
    CommandBuffer(CommandBuffer&& other) noexcept;
    CommandBuffer& operator=(CommandBuffer&& other) noexcept;
    ~CommandBuffer() noexcept;

    void set_viewport(const VkViewport& viewport) noexcept;
    void set_scissor(const VkRect2D& scissor) noexcept;

    // bindings
    void bind(const render::Pipeline& pipeline) noexcept;

    // maybe struct?
    void draw(uint32_t vertex_count, uint32_t instance_count,
        uint32_t first_vertex, uint32_t first_instance) noexcept;

    void exec(const VkRenderPassBeginInfo& begin_info,
        stdx::function_ref<void()> c) noexcept;
    void record(stdx::function_ref<void()> c) noexcept;

    // TODO: find a better way to do this.
    void submit(Operation op_type, stdx::span<VkSemaphore> wait_semaphores,
        stdx::span<VkPipelineStageFlags> wait_for_pipeline_stages,
        stdx::span<VkSemaphore> signal_semaphores, VkFence fence) noexcept;

private:
    void start_record() noexcept;
    void end_record() noexcept;

    void begin_renderpass(const VkRenderPassBeginInfo& begin_info) noexcept;
    void end_renderpass() noexcept;

private:
    DeviceContext* context_;
    underlying_type underlying_;
};
} // namespace zoo::render::scene
