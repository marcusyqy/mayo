
#pragma once
#include "render/resources/Buffer.hpp"
#include "render/scene/CommandBuffer.hpp"
#include "render/sync/Fence.hpp"

namespace zoo::render::scene {

class UploadContext : CommandBuffer {
public:
    void cache(resources::Buffer&& buffer) noexcept;
    void wait() noexcept;
    void submit() noexcept;

    using CommandBuffer::copy;
    using CommandBuffer::transition;
    using CommandBuffer::transition_to_copy;
    using CommandBuffer::transition_to_shader_read;

    UploadContext(DeviceContext& context) noexcept;
    ~UploadContext() noexcept;

private:
    std::vector<resources::Buffer> buffers_;
    sync::Fence fence_;

    bool submitted_;
};

} // namespace zoo::render::scene
