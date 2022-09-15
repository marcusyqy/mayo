#pragma once
#include "core/log.hpp"
#include <functional>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan::debug {

class messenger {
public:
    messenger(VkInstance instance) noexcept;
    ~messenger() noexcept;

    messenger(const messenger&) noexcept = delete;
    auto operator=(const messenger&) noexcept -> messenger& = delete;

    messenger(messenger&& other) noexcept;
    auto operator=(messenger&& other) noexcept -> messenger&;

    auto reset() noexcept -> void;

private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
};

} // namespace zoo::render::vulkan::debug
