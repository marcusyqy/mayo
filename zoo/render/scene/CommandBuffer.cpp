#include "CommandBuffer.hpp"
#include "core/fwd.hpp"

namespace zoo::render::scene {

CommandBuffer::CommandBuffer(ref<DeviceContext> context) noexcept
    : context_{context}, underlying_{context_->buffer_from_pool()} {}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    : context_{std::move(other.context_)}, underlying_{
                                               std::move(other.underlying_)} {
    other.reset();
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
    context_ = std::move(other.context_);
    underlying_ = std::move(other.underlying_);
    other.reset();
    return *this;
}

CommandBuffer::~CommandBuffer() noexcept { reset(); }

void CommandBuffer::reset() noexcept {
    context_.reset();
    underlying_ = nullptr;
}

void CommandBuffer::clear() noexcept { vkResetCommandBuffer(underlying_, 0); }

VkCommandBuffer CommandBuffer::release() noexcept {
    VkCommandBuffer ret = underlying_;
    reset();
    return ret;
}

void CommandBuffer::start_record() noexcept {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    clear();
    VK_EXPECT_SUCCESS(vkBeginCommandBuffer(underlying_, &begin_info),
        [](VkResult /* result */) {})
}

void CommandBuffer::end_record() noexcept {
    VK_EXPECT_SUCCESS(
        vkEndCommandBuffer(underlying_), [](VkResult /* result */) {});
}

void CommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count,
    uint32_t first_vertex, uint32_t first_instance) noexcept {
    vkCmdDraw(underlying_, vertex_count, instance_count, first_vertex,
        first_instance);
}

void CommandBuffer::bind(const render::Pipeline& pipeline) noexcept {
    vkCmdBindPipeline(underlying_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::begin_renderpass(
    const VkRenderPassBeginInfo& begin_info) noexcept {
    vkCmdBeginRenderPass(underlying_, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::end_renderpass() noexcept {
    vkCmdEndRenderPass(underlying_);
}

void CommandBuffer::set_viewport(const VkViewport& viewport) noexcept {
    vkCmdSetViewport(underlying_, 0, 1, std::addressof(viewport));
}

void CommandBuffer::set_scissor(const VkRect2D& scissor) noexcept {
    vkCmdSetScissor(underlying_, 0, 1, std::addressof(scissor));
}

void CommandBuffer::record(stdx::function_ref<void()> c) noexcept {
    start_record();
    c();
    end_record();
}

void CommandBuffer::exec(const VkRenderPassBeginInfo& begin_info,
    stdx::function_ref<void()> c) noexcept {
    begin_renderpass(begin_info);
    c();
    end_renderpass();
}

void CommandBuffer::submit(Operation op_type,
    stdx::span<VkSemaphore> wait_semaphores,
    stdx::span<VkPipelineStageFlags> wait_for_pipeline_stages,
    stdx::span<VkSemaphore> signal_semaphores, VkFence fence) noexcept {

    ZOO_ASSERT(wait_semaphores.size() == wait_for_pipeline_stages.size(),
        "Wait semaphores must contain the same amount of elements as wait for "
        "pipelines stages flags!");

    auto queue = context_->retrieve(op_type);
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount =
        static_cast<uint32_t>(wait_semaphores.size());
    submit_info.pWaitSemaphores = wait_semaphores.data();
    submit_info.pWaitDstStageMask = wait_for_pipeline_stages.data();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &underlying_;
    submit_info.signalSemaphoreCount =
        static_cast<uint32_t>(signal_semaphores.size());
    submit_info.pSignalSemaphores = signal_semaphores.data();

    // TODO: determine if we really need a fence here
    VK_EXPECT_SUCCESS(vkQueueSubmit(queue, 1, &submit_info, fence));
}

} // namespace zoo::render::scene