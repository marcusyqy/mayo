#pragma once

#include "main/application.hpp"
#include "vulkan/device.hpp"
#include <vulkan/vulkan.h>

#include "render/vulkan/debug/messenger.hpp"

namespace zoo::render {

namespace engine_detail {
struct info {
    application::info app_info;
    bool debug_layer;
};
} // namespace engine

class engine {
public:
    using info = engine_detail::info;
    engine(const info& info) noexcept;
    ~engine() noexcept;

    auto initialize() noexcept -> void;
    auto cleanup() noexcept -> void;

    engine(const engine&) noexcept = delete;
    auto operator=(const engine&) noexcept -> engine& = delete;
    engine(engine&&) noexcept = delete;
    auto operator=(engine&&) noexcept -> engine& = delete;

private:
    info info_;

    VkInstance instance_ = VK_NULL_HANDLE;
    std::shared_ptr<vulkan::device> device_ = nullptr;
    std::optional<vulkan::debug::messenger> debugger_ = std::nullopt;
};

} // namespace zoo::render
