#pragma once
#include "core/log.hpp"
#include <functional>
#include <vulkan/vulkan.h>

namespace zoo::render::debug {

class messenger {
public:
    messenger(VkInstance instance) noexcept;
    ~messenger() noexcept;

    messenger(const messenger&) noexcept = delete;
    messenger& operator=(const messenger&) noexcept = delete;

    messenger(messenger&& other) noexcept;
    messenger& operator=(messenger&& other) noexcept;

    void reset() noexcept;

private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
};

} // namespace zoo::render::debug
