#pragma once
#include "core/Log.hpp"
#include <functional>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan::debug {

class Messenger {
public:
    Messenger(VkInstance instance) noexcept;
    ~Messenger() noexcept;

    Messenger(const Messenger&) noexcept = delete;
    auto operator=(const Messenger&) noexcept -> Messenger& = delete;

    Messenger(Messenger&& other) noexcept;
    auto operator=(Messenger&& other) noexcept -> Messenger&;

    auto reset() noexcept -> void;

private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
};

} // namespace zoo::render::vulkan::debug