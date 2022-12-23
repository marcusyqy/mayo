#pragma once

#include "utils/physical_device.hpp"

#include <memory>
#include <string_view>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

class device : std::enable_shared_from_this<device> {
public:
    device(VkInstance instance, utils::physical_device pdevice,
        const utils::queue_family_properties& family_props) noexcept;

    ~device() noexcept;

    device(const device& other) noexcept = delete;
    device(device&& other) noexcept = delete;

    void operator=(const device& other) noexcept = delete;
    void operator=(device&& other) noexcept = delete;
    void reset() noexcept;

    const utils::physical_device& get() const noexcept { return physical_; }

    void release_device_resource(VkFence fence) noexcept;

private:
    utils::physical_device physical_ = nullptr;
    VkDevice logical_ = nullptr;
};

} // namespace zoo::render::vulkan
