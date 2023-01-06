#pragma once
#include "render/device_context.hpp"
#include "render/fwd.hpp"

namespace zoo::render::scene {

class frame {
public:
    struct present_output {
        VkImageView view;
        VkFramebuffer fbo;
    };

public:
    void set_present_output(present_output&& output) noexcept;
    present_output release_present_output() noexcept;

    void reset() noexcept;

    frame(std::shared_ptr<device_context> context) noexcept;
    ~frame() noexcept;

    frame(frame&& frame) noexcept;
    frame& operator=(frame&& frame) noexcept;

    frame(const frame& frame) noexcept = delete;
    frame& operator=(const frame& frame) noexcept = delete;

private:
    std::shared_ptr<device_context> context_;
    present_output present_;
};

} // namespace zoo::render::scene
