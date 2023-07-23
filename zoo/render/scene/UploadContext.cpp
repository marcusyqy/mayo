#include "UploadContext.hpp"

namespace zoo::render::scene {

void UploadContext::cache(resources::Buffer&& buffer) noexcept { buffers_.emplace_back(std::move(buffer)); }
void UploadContext::clear_cache() noexcept { buffers_.clear(); }

UploadContext::UploadContext(DeviceContext& context) noexcept : CommandBuffer(context, Operation::transfer) {}

} // namespace zoo::render::scene
