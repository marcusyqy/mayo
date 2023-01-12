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
    switch (state_) {
    case state::STARTED:
        ZOO_LOG_WARN("Starting a command buffer that has already started!");
        return;
    case state::ENDED:
        ZOO_LOG_WARN(
            "Starting a command buffer that has ended but not submitted!");
        return;
    default:
        break;
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    VK_EXPECT_SUCCESS(vkBeginCommandBuffer(underlying_, &begin_info),
        [this](VkResult /* result */) { state_ = state::UNKNOWN; })

    state_ = state::STARTED;
}

void command_buffer::end_record() noexcept {
    if (state_ == state::STARTED) {
        VK_EXPECT_SUCCESS(vkEndCommandBuffer(underlying_),
            [this](VkResult /* result */) { state_ = state::UNKNOWN; });
        state_ = state::ENDED;
    } else {
        ZOO_LOG_WARN("Ending a command buffer that hasn't started!");
    }
}

void command_buffer::submit() noexcept {
    if (state_ == state::STARTED)
        end_record();

    ZOO_ASSERT(state_ == state::ENDED, "State must end before submission");

    // add submit code here
}

void command_buffer::consume(
    [[maybe_unused]] renderpass render_context) noexcept {}

} // namespace zoo::render::scene
