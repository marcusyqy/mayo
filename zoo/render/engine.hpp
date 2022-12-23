#pragma once

#include <optional>

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

} // namespace engine_detail

class engine {
public:
    using info = engine_detail::info;
    using physical_device_iterator =
        typename std::vector<vulkan::utils::physical_device>::const_iterator;

    engine(const info& info) noexcept;
    ~engine() noexcept;

    void initialize() noexcept;
    void cleanup() noexcept;

    engine(const engine&) noexcept = delete;
    engine& operator=(const engine&) noexcept = delete;
    engine(engine&&) noexcept = delete;
    engine& operator=(engine&&) noexcept = delete;

    const std::vector<vulkan::utils::physical_device>&
    physical_devices() const noexcept {
        return physical_devices_;
    }

    const std::vector<std::shared_ptr<vulkan::device>>&
    devices() const noexcept {
        return devices_;
    }

    std::shared_ptr<vulkan::device> promote(
        physical_device_iterator physical_device) noexcept;

private:
    info info_;

    VkInstance instance_ = nullptr;

    // stores all the physical devices.
    // could be stored in another class that can show more intent
    // and possibly have a better syntax as compared to this.
    std::vector<vulkan::utils::physical_device> physical_devices_{};
    std::vector<std::shared_ptr<vulkan::device>> devices_{};

    // debugger may be named incorrectly
    // TODO: change this name to something that is more correct
    std::optional<vulkan::debug::messenger> debugger_ = std::nullopt;
};

} // namespace zoo::render
