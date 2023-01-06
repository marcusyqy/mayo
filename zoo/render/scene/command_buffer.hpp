#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"

namespace zoo::render::scene {

class command_buffer {
public:
    using underlying_type = VkCommandBuffer;

    underlying_type release() noexcept;
    void reset() noexcept;

    command_buffer(std::shared_ptr<device_context> context) noexcept;
    command_buffer(command_buffer&& other) noexcept;
    command_buffer& operator=(command_buffer&& other) noexcept;
    ~command_buffer() noexcept;

private:
    std::shared_ptr<device_context> context_;
    underlying_type underlying_;
};
} // namespace zoo::render::scene
