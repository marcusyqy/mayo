#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include "core/Log.hpp"

namespace zoo::render::vulkan::debug {

class Messenger {
public:
    Messenger(VkInstance instance) noexcept;
    ~Messenger() noexcept;

    Messenger(const Messenger&) noexcept = delete;
    Messenger& operator=(const Messenger&) noexcept = delete;
    
    Messenger(Messenger&& other) noexcept;
    Messenger& operator=(Messenger&& other) noexcept;

    void reset() noexcept;

private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
};

}