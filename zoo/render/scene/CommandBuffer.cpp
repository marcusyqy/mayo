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

PipelineBindContext& PipelineBindContext::bindings(const Resource_Bindings& binding, stdx::span<u32> offset) noexcept {
    auto set   = binding.sets();
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

PresentContext::PresentContext(
    VkSemaphore image_available,
    VkPipelineStageFlags pipeline_stage_flags,
    VkSemaphore render_done) noexcept :
    image_available_(image_available),
    pipeline_stage_flags_(pipeline_stage_flags), render_done_(render_done) {}

CommandBuffer::CommandBuffer(Device_Context& context, Operation op_type) noexcept :
    context_{ std::addressof(context) }, underlying_{ context_->vk_command_buffer_from_pool(op_type) },
    op_type_(op_type) {}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept :
    context_{ std::move(other.context_) }, underlying_{ std::move(other.underlying_) }, op_type_(other.op_type_) {
    other.reset();
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
    context_    = std::move(other.context_);
    underlying_ = std::move(other.underlying_);
    op_type_    = std::move(other.op_type_);
    other.reset();
    return *this;
}

CommandBuffer::~CommandBuffer() noexcept { reset(); }

void CommandBuffer::reset() noexcept {
    context_    = nullptr;
    underlying_ = nullptr;
    op_type_    = Operation::unknown;
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
    record_status_ = RecordStatus::begin;
}

void CommandBuffer::end_record() noexcept {
    VK_EXPECT_SUCCESS(vkEndCommandBuffer(underlying_), [](VkResult /* result */) {});
    record_status_ = RecordStatus::end;
}

void CommandBuffer::draw(uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) noexcept {
    assure_status(RecordStatus::begin);
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

void CommandBuffer::draw_indexed(uint32_t instance_count) noexcept {
    draw_indexed(instance_count, static_cast<uint32_t>(index_buffer_bind_context_.count_), 0, 0, 0);
}

void CommandBuffer::draw_indexed(
    uint32_t instance_count,
    uint32_t index_count,
    uint32_t first_index,
    uint32_t first_vertex,
    uint32_t first_instance) noexcept {
    assure_status(RecordStatus::begin);
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
        index_count, // static_cast<uint32_t>(index_buffer_bind_context_.count_),
        instance_count,
        first_index,
        first_vertex,
        first_instance);
}

PipelineBindContext CommandBuffer::bind_pipeline(const render::Pipeline& pipeline) noexcept {
    assure_status(RecordStatus::begin);
    vkCmdBindPipeline(underlying_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    return { underlying_, pipeline.get(), pipeline.layout() };
}

void CommandBuffer::bind_vertex_buffers(stdx::span<const render::resources::Buffer> buffers) noexcept {
    assure_status(RecordStatus::begin);
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
    assure_status(RecordStatus::begin);

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
    assure_status(RecordStatus::begin);
    bind_vertex_buffers({ &mesh.vertices(), 1 });
    bind_index_buffer(mesh.indices());
}

void CommandBuffer::begin_renderpass(
    const Framebuffer& rt,
    stdx::span<VkClearValue> clear_colors,
    RenderArea* render_area) noexcept {
    RenderArea default_ra{ .offset = { 0, 0 }, .extent = { rt.width(), rt.height() } };
    if (render_area == nullptr) render_area = &default_ra;

    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass        = rt.renderpass();
    renderpass_info.framebuffer       = rt.get();
    renderpass_info.renderArea.offset = { render_area->offset.x, render_area->offset.y };
    renderpass_info.renderArea.extent = { render_area->extent.x, render_area->extent.y };
    renderpass_info.pNext             = nullptr;

    renderpass_info.clearValueCount = (u32)clear_colors.size();
    renderpass_info.pClearValues    = clear_colors.data();

    begin_renderpass(renderpass_info);
}

void CommandBuffer::begin_renderpass(const VkRenderPassBeginInfo& begin_info) noexcept {
    assure_status(RecordStatus::begin);
    vkCmdBeginRenderPass(underlying_, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::end_renderpass() noexcept {
    // no need to assure command buffer has began since begin renderpass would be called first.
    vkCmdEndRenderPass(underlying_);
}

void CommandBuffer::set_viewport(const VkViewport& viewport) noexcept {
    assure_status(RecordStatus::begin);
    vkCmdSetViewport(underlying_, 0, 1, std::addressof(viewport));
}

void CommandBuffer::set_scissor(const VkRect2D& scissor) noexcept {
    assure_status(RecordStatus::begin);
    vkCmdSetScissor(underlying_, 0, 1, std::addressof(scissor));
}

void CommandBuffer::assure_status(RecordStatus status) {
    if (status == record_status_) return;

    switch (status) {
        case RecordStatus::begin: start_record(); break;
        case RecordStatus::end: end_record(); break;
    }
    record_status_ = status;
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
    stdx::span<VkSemaphore> wait_semaphores,
    stdx::span<VkPipelineStageFlags> wait_for_pipeline_stages,
    stdx::span<VkSemaphore> signal_semaphores,
    VkFence fence) noexcept {
    assure_status(RecordStatus::end);

    ZOO_ASSERT(
        wait_semaphores.size() == wait_for_pipeline_stages.size(),
        "Wait semaphores must contain the same amount of elements as wait for "
        "pipelines stages flags!");

    auto queue = context_->retrieve(op_type_);
    VkSubmitInfo submit_info{};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = static_cast<u32>(wait_semaphores.size());
    submit_info.pWaitSemaphores      = wait_semaphores.data();
    submit_info.pWaitDstStageMask    = wait_for_pipeline_stages.data();
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &underlying_;
    submit_info.signalSemaphoreCount = static_cast<u32>(signal_semaphores.size());
    submit_info.pSignalSemaphores    = signal_semaphores.data();

    // TODO: determine if we really need a fence here
    VK_EXPECT_SUCCESS(vkQueueSubmit(queue, 1, &submit_info, fence));
}

void CommandBuffer::submit(const PresentContext& present_context, VkFence fence) noexcept {
    VkSemaphore wait_semaphores[]      = { present_context.image_available_ };
    VkPipelineStageFlags wait_stages[] = { present_context.pipeline_stage_flags_ };
    VkSemaphore signal_semaphores[]    = { present_context.render_done_ };
    submit(wait_semaphores, wait_stages, signal_semaphores, fence);
}

void CommandBuffer::copy(const render::resources::Buffer& from, render::resources::Buffer& to) noexcept {
    assure_status(RecordStatus::begin);
    ZOO_ASSERT(from.allocated_size() <= to.allocated_size(), "Must be the same size or more for the buffer copying to");
    // TODO: if this should be safe then we should find the min of both?
    VkBufferCopy copy{ .srcOffset = 0, .dstOffset = 0, .size = from.allocated_size() };
    vkCmdCopyBuffer(underlying_, from.handle(), to.handle(), 1, &copy);
}

void CommandBuffer::transition_to_copy(render::resources::Texture& texture) noexcept {
    transition(
        texture,
        texture.layout(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        texture.access_flags(),
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel   = 0,
          .levelCount     = texture.mip_level(),
          .baseArrayLayer = 0,
          .layerCount     = texture.array_count() });
}

void CommandBuffer::transition_to_shader_read(resources::Texture& texture) noexcept {
    transition(
        texture,
        texture.layout(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        texture.access_flags(),
        VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, // TODO: this needs to find out some shit.
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel   = 0,
          .levelCount     = texture.mip_level(),
          .baseArrayLayer = 0,
          .layerCount     = texture.array_count() });
}

void CommandBuffer::copy(const render::resources::Buffer& from, render::resources::Texture& to) noexcept {
    assure_status(RecordStatus::begin);
    ZOO_ASSERT(
        from.allocated_size() <= to.allocated_size(),
        "Must be the same size or more for the texture copying to");

    auto image_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    if (to.layout() != image_layout) transition_to_copy(to);

    VkBufferImageCopy copy{ .bufferOffset      = 0,
                            .bufferRowLength   = 0,
                            .bufferImageHeight = 0,
                            .imageSubresource  = {
                                // TODO : change based on format
                                                   .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                                   .mipLevel       = 0, // to.mip_level(),
                                                   .baseArrayLayer = 0,
                                                   .layerCount = to.array_count(),
                                                },
                            .imageExtent    = to.extent()
    };
    vkCmdCopyBufferToImage(underlying_, from.handle(), to.handle(), image_layout, 1, &copy);
}

void CommandBuffer::transition(
    render::resources::Texture& texture,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkAccessFlags src_access,
    VkAccessFlags dst_access,
    VkPipelineStageFlags start_pipeline_stage,
    VkPipelineStageFlags end_pipeline_stage,
    VkImageSubresourceRange range) noexcept {

    VkImageMemoryBarrier image_barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask    = src_access,
        .dstAccessMask    = dst_access,
        .oldLayout        = old_layout,
        .newLayout        = new_layout,
        .image            = texture.handle(),
        .subresourceRange = range,
    };

    // @TODO: maybe batch this.
    // barrier the image into the transfer-receive layout
    vkCmdPipelineBarrier(
        underlying_,
        start_pipeline_stage,
        end_pipeline_stage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &image_barrier);

    texture.layout(new_layout);
    texture.access_flags(dst_access);
}

} // namespace zoo::render::scene
