#include "command_buffer.hpp"

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
} // namespace zoo::render::scene
