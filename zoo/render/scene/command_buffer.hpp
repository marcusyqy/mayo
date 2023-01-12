#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "render/renderpass.hpp"

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

    void start_record() noexcept;
    void end_record() noexcept;
    void submit() noexcept;

    void set_viewport(const VkViewport& viewport) noexcept;
    void set_scissor(const VkRect2D& scissor) noexcept;

    void consume(renderpass render_context) noexcept;

private:
    std::shared_ptr<device_context> context_;
    underlying_type underlying_;

    enum class state { UNKNOWN, STARTED, ENDED, SUBMITTED } state_;
};
} // namespace zoo::render::scene
