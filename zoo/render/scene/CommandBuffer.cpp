#include "CommandBuffer.hpp"
#include "core/fwd.hpp"

namespace zoo::render::scene {

namespace {
const auto default_index_type = VK_INDEX_TYPE_UINT32;

VkIndexType size_to_index_type(size_t size) noexcept {
    switch (size) {
        case 1: return VK_INDEX_TYPE_UINT8_EXT;
        case 2: return VK_INDEX_TYPE_UINT16;
        case 4: return VK_INDEX_TYPE_UINT32;
        default:
            ZOO_LOG_ERROR("[index type not recognised!] Defaulting to the largest {}", size);
            return default_index_type;
    }
}

} // namespace

PipelineBindContext& PipelineBindContext::push_constants(const PushConstant& constant, void* data) noexcept {
    vkCmdPushConstants(cmd_buffer_, pipeline_layout_, constant.stageFlags, constant.offset, constant.size, data);
    return *this;
}

PipelineBindContext& PipelineBindContext::bindings(const ResourceBindings& binding, stdx::span<u32> offset) noexcept {
    auto set = binding.sets();
    auto count = binding.count();

    // TODO: change this when compute exists bind point
    vkCmdBindDescriptorSets(
        cmd_buffer_,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout_,
        0,
        count,
        set,
        static_cast<u32>(offset.size()),
        offset.data());
    return *this;
}

PipelineBindContext::PipelineBindContext(
    VkCommandBuffer cmd_buffer,
    VkPipeline pipeline,
    VkPipelineLayout pipeline_layout) noexcept :
    cmd_buffer_(cmd_buffer),
    pipeline_(pipeline), pipeline_layout_(pipeline_layout) {}

CommandBuffer::CommandBuffer(DeviceContext& context) noexcept :
    context_{ std::addressof(context) }, underlying_{ context_->vk_command_buffer_from_pool() } {}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept :
    context_{ std::move(other.context_) }, underlying_{ std::move(other.underlying_) } {
    other.reset();
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
    context_    = std::move(other.context_);
    underlying_ = std::move(other.underlying_);
    other.reset();
    return *this;
}

CommandBuffer::~CommandBuffer() noexcept { reset(); }

void CommandBuffer::reset() noexcept {
    context_    = nullptr;
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
    begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags            = 0; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    clear();
    VK_EXPECT_SUCCESS(vkBeginCommandBuffer(underlying_, &begin_info), [](VkResult /* result */) {})
}

void CommandBuffer::end_record() noexcept {
    VK_EXPECT_SUCCESS(vkEndCommandBuffer(underlying_), [](VkResult /* result */) {});
}

void CommandBuffer::draw(uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) noexcept {
    if (vertex_buffer_bind_context_.buffers_.empty()) {
        ZOO_LOG_ERROR("`draw` called without `bind_vertex_buffers`");
        return;
    }
    vkCmdDraw(
        underlying_,
        static_cast<uint32_t>(vertex_buffer_bind_context_.count_),
        instance_count,
        first_vertex,
        first_instance);
}

void CommandBuffer::draw_indexed(
    uint32_t instance_count,
    uint32_t first_index,
    uint32_t first_vertex,
    uint32_t first_instance) noexcept {
    if (vertex_buffer_bind_context_.buffers_.empty()) {
        ZOO_LOG_ERROR("`draw_indexed` called without `bind_vertex_buffers`");
        return;
    }

    if (index_buffer_bind_context_.buffer_ == nullptr) {
        ZOO_LOG_ERROR("`draw_indexed` called without `bind_index_buffer`");
        return;
    }
    vkCmdDrawIndexed(
        underlying_,
        static_cast<uint32_t>(index_buffer_bind_context_.count_),
        instance_count,
        first_index,
        first_vertex,
        first_instance);
}

PipelineBindContext CommandBuffer::bind_pipeline(const render::Pipeline& pipeline) noexcept {
    vkCmdBindPipeline(underlying_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    return { underlying_, pipeline.get(), pipeline.layout() };
}

void CommandBuffer::bind_vertex_buffers(stdx::span<const render::resources::Buffer> buffers) noexcept {
    const auto size = buffers.size();
    auto& vbbuffers = vertex_buffer_bind_context_.buffers_;
    vbbuffers.clear();
    vbbuffers.reserve(size);

    auto& vboffsets = vertex_buffer_bind_context_.offsets_;
    vboffsets.clear();
    vboffsets.reserve(size);

    auto& vbcounts = vertex_buffer_bind_context_.count_;
    vbcounts       = std::numeric_limits<size_t>::max();

    for (const auto& x : buffers) {
        vbbuffers.emplace_back(x.handle());
        vboffsets.emplace_back(x.offset());
        vbcounts = std::min(vbcounts, x.count());
    }

    vkCmdBindVertexBuffers(underlying_, 0, static_cast<uint32_t>(size), vbbuffers.data(), vboffsets.data());
}

void CommandBuffer::bind_index_buffer(const render::resources::Buffer& ib) noexcept {

    index_buffer_bind_context_.buffer_     = ib.handle();
    index_buffer_bind_context_.offset_     = 0;
    index_buffer_bind_context_.index_type_ = size_to_index_type(ib.object_size());
    index_buffer_bind_context_.count_      = ib.count();

    vkCmdBindIndexBuffer(
        underlying_,
        index_buffer_bind_context_.buffer_,
        index_buffer_bind_context_.offset_,
        index_buffer_bind_context_.index_type_);
}

void CommandBuffer::bind_mesh(const render::resources::Mesh& mesh) noexcept {
    bind_vertex_buffers({ &mesh.vertices(), 1 });
    bind_index_buffer(mesh.indices());
}

void CommandBuffer::begin_renderpass(const VkRenderPassBeginInfo& begin_info) noexcept {
    vkCmdBeginRenderPass(underlying_, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::end_renderpass() noexcept { vkCmdEndRenderPass(underlying_); }

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

void CommandBuffer::exec(const VkRenderPassBeginInfo& begin_info, stdx::function_ref<void()> c) noexcept {
    begin_renderpass(begin_info);
    c();
    end_renderpass();
}

void CommandBuffer::clear_context() noexcept {
    vertex_buffer_bind_context_.count_ = 0;
    vertex_buffer_bind_context_.buffers_.clear();
    vertex_buffer_bind_context_.offsets_.clear();

    index_buffer_bind_context_.buffer_     = nullptr;
    index_buffer_bind_context_.offset_     = 0;
    index_buffer_bind_context_.index_type_ = default_index_type;
    index_buffer_bind_context_.count_      = 0;
}

void CommandBuffer::submit(
    Operation op_type,
    stdx::span<VkSemaphore> wait_semaphores,
    stdx::span<VkPipelineStageFlags> wait_for_pipeline_stages,
    stdx::span<VkSemaphore> signal_semaphores,
    VkFence fence) noexcept {

    ZOO_ASSERT(
        wait_semaphores.size() == wait_for_pipeline_stages.size(),
        "Wait semaphores must contain the same amount of elements as wait for "
        "pipelines stages flags!");

    auto queue = context_->retrieve(op_type);
    VkSubmitInfo submit_info{};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = static_cast<uint32_t>(wait_semaphores.size());
    submit_info.pWaitSemaphores      = wait_semaphores.data();
    submit_info.pWaitDstStageMask    = wait_for_pipeline_stages.data();
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &underlying_;
    submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
    submit_info.pSignalSemaphores    = signal_semaphores.data();

    // TODO: determine if we really need a fence here
    VK_EXPECT_SUCCESS(vkQueueSubmit(queue, 1, &submit_info, fence));
}

} // namespace zoo::render::scene
