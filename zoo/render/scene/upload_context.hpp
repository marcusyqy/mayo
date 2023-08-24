
#pragma once
#include "render/resources/buffer.hpp"
#include "render/scene/command_buffer.hpp"
#include "render/sync/fence.hpp"

namespace zoo::render::scene {

class Upload_Context : Command_Buffer {
public:
    void cache(resources::Buffer&& buffer) noexcept;
    void wait() noexcept;
    void submit() noexcept;

    using Command_Buffer::copy;
    using Command_Buffer::transition;
    using Command_Buffer::transition_to_copy;
    using Command_Buffer::transition_to_shader_read;

    Upload_Context(Device_Context& context) noexcept;
    Upload_Context() noexcept = default;
    ~Upload_Context() noexcept;

    Upload_Context(const Upload_Context&) noexcept            = delete;
    Upload_Context& operator=(const Upload_Context&) noexcept = delete;

    Upload_Context(Upload_Context&& o) noexcept;
    Upload_Context& operator=(Upload_Context&& o) noexcept;

private:
    std::vector<resources::Buffer> buffers_ = {};
    sync::Fence fence_                      = {};
    bool submitted_                         = false;
};

} // namespace zoo::render::scene
