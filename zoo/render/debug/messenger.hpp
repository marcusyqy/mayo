#pragma once
#include "core/Log.hpp"
#include <functional>
#include <vulkan/vulkan.h>

namespace zoo::render::debug {

class Messenger {
public:
    Messenger(VkInstance instance) noexcept;
    ~Messenger() noexcept;

    Messenger(const Messenger&) noexcept            = delete;
    Messenger& operator=(const Messenger&) noexcept = delete;

    Messenger(Messenger&& other) noexcept;
    Messenger& operator=(Messenger&& other) noexcept;

    void reset() noexcept;

private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
};

} // namespace zoo::render::debug
