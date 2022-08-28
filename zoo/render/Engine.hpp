#pragma once

#include "vulkan/Device.hpp"
#include <vulkan/vulkan.h>

namespace zoo::render {

class Engine {
public:
    void initialize() noexcept;
    void cleanup() noexcept;

private:
    void create_instance() noexcept;
    void create_device() noexcept;

private:
    VkInstance instance_;
    std::shared_ptr<vulkan::Device> device_;
};

} // namespace zoo::render