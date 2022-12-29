#pragma once

#include "utils/physical_device.hpp"

#include "core/platform/query.hpp"
#include <memory>
#include <string_view>
#include <vulkan/vulkan.h>

namespace zoo::render {

class device : std::enable_shared_from_this<device> {
public:
    device(VkInstance instance, utils::physical_device pdevice,
        const utils::queue_family_properties& family_props,
        const platform::render::query& query) noexcept;

    ~device() noexcept;

    device(const device& other) noexcept = delete;
    device(device&& other) noexcept = delete;

    device& operator=(const device& other) noexcept = delete;
    device& operator=(device&& other) noexcept = delete;

    void reset() noexcept;

    operator const utils::physical_device&() const noexcept { return get(); }
    const utils::physical_device& get() const noexcept { return physical_; }

    void release_device_resource(VkFence fence) noexcept;

private:
    utils::physical_device physical_ = nullptr;
    VkDevice logical_ = nullptr;
    VkQueue queue_ = nullptr;
};

} // namespace zoo::render
