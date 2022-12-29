#pragma once

#include <optional>

#include "device.hpp"
#include "main/application.hpp"
#include <vulkan/vulkan.h>

#include "render/debug/messenger.hpp"

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
        typename std::vector<utils::physical_device>::const_iterator;

    engine(const info& info) noexcept;
    ~engine() noexcept;

    void initialize() noexcept;
    void cleanup() noexcept;

    engine(const engine&) noexcept = delete;
    engine& operator=(const engine&) noexcept = delete;
    engine(engine&&) noexcept = delete;
    engine& operator=(engine&&) noexcept = delete;

    const std::vector<utils::physical_device>&
    physical_devices() const noexcept {
        return physical_devices_;
    }

    const std::vector<std::shared_ptr<device>>& devices() const noexcept {
        return devices_;
    }

    std::shared_ptr<device> promote(physical_device_iterator physical_device,
        utils::queue_family_properties family_props) noexcept;

    VkInstance vk_instance() const noexcept { return instance_; }

private:
    info info_;

    VkInstance instance_ = nullptr;

    // stores all the physical devices.
    // could be stored in another class that can show more intent
    // and possibly have a better syntax as compared to this.
    std::vector<utils::physical_device> physical_devices_{};
    std::vector<std::shared_ptr<device>> devices_{};

    // debugger may be named incorrectly
    // TODO: change this name to something that is more correct
    std::optional<debug::messenger> debugger_ = std::nullopt;
};

} // namespace zoo::render
