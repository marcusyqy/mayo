#pragma once

#include <optional>

#include "device_context.hpp"
#include "fwd.hpp"
#include "utils/physical_device.hpp"

#include "render/debug/messenger.hpp"

namespace zoo::render {

namespace engine {

struct Info {
    bool debug_layer;
};

} // namespace engine

// this class should be just to query for properties that are related to
// rendering.
// all other rendering logic should be in `Device_Context`
class Engine { // @TODO: we should probably think about whether we want to group this with `Device_Context`
public:
    using Info                     = engine::Info;
    using physical_device_iterator = typename std::vector<utils::Physical_Device>::const_iterator;

    Engine(const Info& info = { .debug_layer = true }) noexcept;
    ~Engine() noexcept;

    Engine(const Engine&) noexcept            = delete;
    Engine& operator=(const Engine&) noexcept = delete;
    Engine(Engine&&) noexcept                 = delete;
    Engine& operator=(Engine&&) noexcept      = delete;

    const std::vector<utils::Physical_Device>& physical_devices() const noexcept { return physical_devices_; }

    VkInstance vk_instance() const noexcept { return instance_; }

    // TODO : write allocator
    const VkAllocationCallbacks* allocator() const noexcept {
        return callbacks_ ? std::addressof(callbacks_.value()) : nullptr;
    }

    Device_Context& context() noexcept { return context_; }
    const Device_Context& context() const noexcept { return context_; }

private:
    Info info_;

    VkInstance instance_ = nullptr;

    // stores all the physical devices.
    // could be stored in another class that can show more intent
    // and possibly have a better syntax as compared to this.
    std::vector<utils::Physical_Device> physical_devices_{};
    Device_Context context_;

    // debugger may be named incorrectly
    // TODO: change this name to something that is more correct
    std::optional<debug::Messenger> reporter_ = std::nullopt;

    std::optional<VkAllocationCallbacks> callbacks_ = std::nullopt;
};

} // namespace zoo::render
