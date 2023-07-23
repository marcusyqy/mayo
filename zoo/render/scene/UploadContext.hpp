
#pragma once
#include "render/resources/Buffer.hpp"
#include "render/scene/CommandBuffer.hpp"

namespace zoo::render::scene {

class UploadContext : CommandBuffer {
public:
    void cache(resources::Buffer&& buffer) noexcept;
    void clear_cache() noexcept;

    using CommandBuffer::copy;
    using CommandBuffer::submit;

    UploadContext(DeviceContext& context) noexcept;

private:
    std::vector<resources::Buffer> buffers_;
};

} // namespace zoo::render::scene
