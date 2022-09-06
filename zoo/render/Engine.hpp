#pragma once

#include "main/Info.hpp"
#include "vulkan/Device.hpp"
#include <vulkan/vulkan.h>

#include "render/vulkan/debug/Messenger.hpp"

namespace zoo::render {

namespace engine {
struct Info {
    application::Info app_info_;
    bool debug_layer_;
};
} // namespace engine

class Engine {
public:
    Engine(const engine::Info& info) noexcept;
    ~Engine() noexcept;

    void initialize() noexcept;
    void cleanup() noexcept;

    Engine(const Engine&) noexcept = delete;
    Engine& operator=(const Engine&) noexcept = delete;
    Engine(Engine&&) noexcept = delete;
    Engine& operator=(Engine&&) noexcept = delete;

private:
    engine::Info info_;

    VkInstance instance_ = VK_NULL_HANDLE;
    std::shared_ptr<vulkan::Device> device_ = nullptr;
    std::optional<vulkan::debug::Messenger> debugger_ = std::nullopt;
};

} // namespace zoo::render