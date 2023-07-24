
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
    using CommandBuffer::transition;
    using CommandBuffer::transition_to_copy;
    using CommandBuffer::transition_to_shader_read;

    UploadContext(DeviceContext& context) noexcept;

private:
    std::vector<resources::Buffer> buffers_;
};

} // namespace zoo::render::scene
