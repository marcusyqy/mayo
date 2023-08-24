#include "upload_context.hpp"

namespace zoo::render::scene {

void Upload_Context::cache(resources::Buffer&& buffer) noexcept { buffers_.emplace_back(std::move(buffer)); }

void Upload_Context::wait() noexcept {
    if (submitted_) {
        fence_.wait();
        fence_.reset();
        buffers_.clear();
        submitted_ = false;
    }
}
void Upload_Context::submit() noexcept {
    Command_Buffer::submit(nullptr, nullptr, nullptr, fence_);
    submitted_ = true;
}

Upload_Context::Upload_Context(Device_Context& context) noexcept :
    Command_Buffer(context, Operation::transfer), fence_(context), submitted_(false) {}

Upload_Context::~Upload_Context() noexcept {
    if (fence_.valid()) wait();
}

Upload_Context::Upload_Context(Upload_Context&& o) noexcept { *this = std::move(o); }

Upload_Context& Upload_Context::operator=(Upload_Context&& o) noexcept {
    Command_Buffer::operator=(std::move(o));
    buffers_   = std::move(o.buffers_);
    fence_     = std::move(o.fence_);
    submitted_ = o.submitted_;
    return *this;
}

} // namespace zoo::render::scene
