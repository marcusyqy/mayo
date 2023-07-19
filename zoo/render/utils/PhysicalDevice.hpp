#pragma once
#include "QueueFamilyProperties.hpp"
#include "render/fwd.hpp"

#include <string_view>
#include <unordered_set>
#include <vector>

namespace zoo::render::utils {

class PhysicalDevice {
public:
    using underlying_type = VkPhysicalDevice;
    using id_type         = uint32_t;

    operator underlying_type() const noexcept { return get(); }
    underlying_type get() const noexcept { return underlying_; }

    PhysicalDevice(underlying_type underlying) noexcept;

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

    [[nodiscard]] const std::vector<QueueFamilyProperties>& queue_properties() const noexcept {
        return queue_family_properties_;
    }

    [[nodiscard]] bool has_present(const QueueFamilyProperties& family_props, VkInstance instance) const noexcept;

    bool has_required_extension(std::string_view extension_name) const noexcept;

private:
    void query_properties_and_features() noexcept;

private:
    VkPhysicalDevice underlying_ = nullptr;

    VkPhysicalDeviceProperties properties_{};
    VkPhysicalDeviceFeatures features_{};
    std::vector<QueueFamilyProperties> queue_family_properties_{};
    std::unordered_set<std::string_view> device_extensions_{};
};

} // namespace zoo::render::utils
