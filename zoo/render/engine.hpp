#pragma once

#include <optional>

#include "main/application.hpp"
#include "utils/physical_device.hpp"
#include <vulkan/vulkan.h>

#include "render/debug/messenger.hpp"

namespace zoo::render {

namespace engine_detail {

struct info {
    application::info app_info;
    bool debug_layer;
};

} // namespace engine_detail

// this class should be just to query for properties that are related to
// rendering
//
// all other rendering logic should be in `device_context`
class engine {
public:
    using info = engine_detail::info;
    using physical_device_iterator =
        typename std::vector<utils::physical_device>::const_iterator;

    engine(const info& info) noexcept;
    ~engine() noexcept;

    engine(const engine&) noexcept = delete;
    engine& operator=(const engine&) noexcept = delete;
    engine(engine&&) noexcept = delete;
    engine& operator=(engine&&) noexcept = delete;

    const std::vector<utils::physical_device>&
    physical_devices() const noexcept {
        return physical_devices_;
    }

    VkInstance vk_instance() const noexcept { return instance_; }

private:
    void initialize() noexcept;
    void cleanup() noexcept;

private:
    info info_;

    VkInstance instance_ = nullptr;

    // stores all the physical devices.
    // could be stored in another class that can show more intent
    // and possibly have a better syntax as compared to this.
    std::vector<utils::physical_device> physical_devices_{};

    // debugger may be named incorrectly
    // TODO: change this name to something that is more correct
    std::optional<debug::messenger> debugger_ = std::nullopt;
};

} // namespace zoo::render
