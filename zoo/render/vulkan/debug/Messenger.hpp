#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include "core/Log.hpp"

namespace zoo::render::vulkan::debug {

class Messenger {
public:
    Messenger(VkInstance instance);
    ~Messenger();

private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
};

}