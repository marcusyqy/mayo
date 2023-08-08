#include "UploadContext.hpp"

namespace zoo::render::scene {

void UploadContext::cache(resources::Buffer&& buffer) noexcept { buffers_.emplace_back(std::move(buffer)); }
void UploadContext::wait() noexcept {
    fence_.wait();
    fence_.reset();
    buffers_.clear();
}
void UploadContext::submit() noexcept { CommandBuffer::submit(nullptr, nullptr, nullptr, fence_); }

UploadContext::UploadContext(DeviceContext& context) noexcept :
    CommandBuffer(context, Operation::transfer), fence_(context) {}

UploadContext::~UploadContext() noexcept {
    wait();
}

} // namespace zoo::render::scene
