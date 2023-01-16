#include "command_buffer.hpp"
#include "core/fwd.hpp"

namespace zoo::render::scene {

command_buffer::command_buffer(std::shared_ptr<device_context> context) noexcept
    : context_{context}, underlying_{context_->buffer_from_pool()} {}

command_buffer::command_buffer(command_buffer&& other) noexcept
    : context_{std::move(other.context_)}, underlying_{
                                               std::move(other.underlying_)} {
    other.reset();
}

command_buffer& command_buffer::operator=(command_buffer&& other) noexcept {
    context_ = std::move(other.context_);
    underlying_ = std::move(other.underlying_);
    other.reset();
    return *this;
}

command_buffer::~command_buffer() noexcept { reset(); }

void command_buffer::reset() noexcept {
    context_.reset();
    underlying_ = nullptr;
}

VkCommandBuffer command_buffer::release() noexcept {
    VkCommandBuffer ret = underlying_;
    reset();
    return ret;
}

void command_buffer::start_record() noexcept {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    VK_EXPECT_SUCCESS(vkBeginCommandBuffer(underlying_, &begin_info),
        [](VkResult /* result */) {})
}

void command_buffer::end_record() noexcept {
    VK_EXPECT_SUCCESS(
        vkEndCommandBuffer(underlying_), [](VkResult /* result */) {});
}

void command_buffer::submit() noexcept {
    // add submit code here
}

void command_buffer::draw(uint32_t vertex_count, uint32_t instance_count,
    uint32_t first_vertex, uint32_t first_instance) noexcept {
    vkCmdDraw(underlying_, vertex_count, instance_count, first_vertex,
        first_instance);
}

void command_buffer::bind(const render::pipeline& pipeline) noexcept {
    vkCmdBindPipeline(underlying_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void command_buffer::begin_renderpass(
    const VkRenderPassBeginInfo& begin_info) noexcept {
    vkCmdBeginRenderPass(underlying_, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void command_buffer::end_renderpass() noexcept {
    vkCmdEndRenderPass(underlying_);
}

} // namespace zoo::render::scene
