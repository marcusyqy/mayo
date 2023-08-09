#include "UploadContext.hpp"

namespace zoo::render::scene {

void UploadContext::cache(resources::Buffer&& buffer) noexcept { buffers_.emplace_back(std::move(buffer)); }

void UploadContext::wait() noexcept {
    if (submitted_) {
        fence_.wait();
        fence_.reset();
        buffers_.clear();
        submitted_ = false;
    }
}
void UploadContext::submit() noexcept {
    CommandBuffer::submit(nullptr, nullptr, nullptr, fence_);
    submitted_ = true;
}

UploadContext::UploadContext(DeviceContext& context) noexcept :
    CommandBuffer(context, Operation::transfer), fence_(context), submitted_(false) {}

UploadContext::~UploadContext() noexcept {
    if (fence_.valid()) wait();
}

UploadContext::UploadContext(UploadContext&& o) noexcept { *this = std::move(o); }

UploadContext& UploadContext::operator=(UploadContext&& o) noexcept {
    CommandBuffer::operator=(std::move(o));
    buffers_   = std::move(o.buffers_);
    fence_     = std::move(o.fence_);
    submitted_ = o.submitted_;
    return *this;
}

} // namespace zoo::render::scene
