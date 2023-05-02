#pragma once
#include "render/DeviceContext.hpp"
#include "render/Pipeline.hpp"
#include "render/RenderPass.hpp"
#include "render/fwd.hpp"
#include "render/resources/Buffer.hpp"
#include "render/resources/Mesh.hpp"
#include "stdx/function_ref.hpp"
#include "zoo.hpp"

namespace zoo::render::scene {

struct PipelineBindContext {
public:
    PipelineBindContext& push_constants(
        const PushConstant& constant, void* data) noexcept;

    PipelineBindContext(VkCommandBuffer cmd_buffer, VkPipeline pipeline,
        VkPipelineLayout pipeline_layout) noexcept;

private:
    VkCommandBuffer cmd_buffer_;
    VkPipeline pipeline_;
    VkPipelineLayout pipeline_layout_;
};

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
    PipelineBindContext bind_pipeline(
        const render::Pipeline& pipeline) noexcept;

    void bind_vertex_buffers(
        stdx::span<const render::resources::Buffer> buffers) noexcept;
    void bind_index_buffer(const render::resources::Buffer& ib) noexcept;
    void bind_mesh(const render::resources::Mesh& mesh) noexcept;

    void draw(uint32_t instance_count, uint32_t first_vertex = 0,
        uint32_t first_instance = 0) noexcept;

    void draw_indexed(uint32_t instance_count, uint32_t first_index = 0,
        uint32_t first_vertex = 0, uint32_t first_instance = 0) noexcept;

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

    void clear_context() noexcept;

private:
    DeviceContext* context_;
    underlying_type underlying_;

    struct VertexBufferBindContext {
        std::vector<VkBuffer> buffers_;
        std::vector<VkDeviceSize> offsets_;
        size_t count_;
    } vertex_buffer_bind_context_;

    struct IndexBufferBindContext {
        VkBuffer buffer_;
        VkDeviceSize offset_;
        VkIndexType index_type_;
        size_t count_;
    } index_buffer_bind_context_;
};
} // namespace zoo::render::scene
