#pragma once

#include <optional>

#include "DeviceContext.hpp"
#include "fwd.hpp"
#include "main/Info.hpp"
#include "utils/PhysicalDevice.hpp"

#include "render/debug/Messenger.hpp"

namespace zoo::render {

namespace engine {

struct Info {
    application::Info app_info;
    bool debug_layer;
};

} // namespace engine

// this class should be just to query for properties that are related to
// rendering.
// all other rendering logic should be in `DeviceContext`
class Engine {
public:
    using Info = engine::Info;
    using physical_device_iterator =
        typename std::vector<utils::PhysicalDevice>::const_iterator;

    Engine(const Info& info) noexcept;
    ~Engine() noexcept;

    Engine(const Engine&) noexcept = delete;
    Engine& operator=(const Engine&) noexcept = delete;
    Engine(Engine&&) noexcept = delete;
    Engine& operator=(Engine&&) noexcept = delete;

    const std::vector<utils::PhysicalDevice>&
    physical_devices() const noexcept {
        return physical_devices_;
    }

    VkInstance vk_instance() const noexcept { return instance_; }

    // TODO : write allocator
    const VkAllocationCallbacks* allocator() const noexcept {
        return callbacks_ ? std::addressof(callbacks_.value()) : nullptr;
    }

    DeviceContext& context() noexcept { return context_; }
    const DeviceContext& context() const noexcept { return context_; }

private:
    Info info_;

    VkInstance instance_ = nullptr;

    // stores all the physical devices.
    // could be stored in another class that can show more intent
    // and possibly have a better syntax as compared to this.
    std::vector<utils::PhysicalDevice> physical_devices_{};
    DeviceContext context_;

    // debugger may be named incorrectly
    // TODO: change this name to something that is more correct
    std::optional<debug::Messenger> debugger_ = std::nullopt;

    std::optional<VkAllocationCallbacks> callbacks_ = std::nullopt;
};

} // namespace zoo::render
