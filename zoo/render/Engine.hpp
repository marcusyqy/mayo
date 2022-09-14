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

    auto initialize() noexcept -> void;
    auto cleanup() noexcept -> void;

    Engine(const Engine&) noexcept = delete;
    auto operator=(const Engine&) noexcept -> Engine& = delete;
    Engine(Engine&&) noexcept = delete;
    auto operator=(Engine&&) noexcept -> Engine& = delete;

private:
    engine::Info info_;

    VkInstance instance_ = VK_NULL_HANDLE;
    std::shared_ptr<vulkan::Device> device_ = nullptr;
    std::optional<vulkan::debug::Messenger> debugger_ = std::nullopt;
};

} // namespace zoo::render