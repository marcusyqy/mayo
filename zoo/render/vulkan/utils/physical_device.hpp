#pragma once
#include "queue_family_properties.hpp"

#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan::utils {

class physical_device {
public:
    using underlying_type = VkPhysicalDevice;
    using id_type = uint32_t;

    operator underlying_type() const noexcept { return underlying_; }
    underlying_type get() const noexcept { return underlying_; }

    physical_device(underlying_type underlying) noexcept;

    // device type
    [[nodiscard]] bool is_discrete() const noexcept;
    [[nodiscard]] bool is_integrated() const noexcept;
    [[nodiscard]] bool is_virtual() const noexcept;
    [[nodiscard]] bool is_cpu() const noexcept;
    [[nodiscard]] bool is_other() const noexcept;

    std::string_view name() const noexcept;
    id_type id() const noexcept;

    VkPhysicalDeviceLimits limits() const noexcept;

    const VkPhysicalDeviceFeatures& features() const noexcept;

    // add device features
    [[nodiscard]] bool has_geometry_shader() const noexcept;

    [[nodiscard]] const std::vector<queue_family_properties>&
    queue_properties() const noexcept {
        return queue_family_properties_;
    }

private:
    void query_properties_and_features() noexcept;

private:
    VkPhysicalDevice underlying_ = nullptr;

    VkPhysicalDeviceProperties properties_{};
    VkPhysicalDeviceFeatures features_{};
    std::vector<queue_family_properties> queue_family_properties_{};
};

} // namespace zoo::render::vulkan::utils
