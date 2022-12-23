#include "physical_device.hpp"
#include <algorithm>
#include <iterator>

namespace zoo::render::vulkan::utils {

physical_device::physical_device(underlying_type underlying) noexcept
    : underlying_{underlying} {}

void physical_device::query_properties_and_features() noexcept {
    vkGetPhysicalDeviceProperties(underlying_, &properties_);
    vkGetPhysicalDeviceFeatures(underlying_, &features_);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        underlying_, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        underlying_, &queue_family_count, queue_families.data());

    uint32_t idx{};
    std::transform(std::begin(queue_families), std::end(queue_families),
        std::back_inserter(queue_family_properties_),
        [&idx](
            const VkQueueFamilyProperties& props) -> queue_family_properties {
            return {idx++, props};
        });
}

bool physical_device::is_discrete() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}
bool physical_device::is_integrated() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}
bool physical_device::is_virtual() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
}
bool physical_device::is_cpu() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
}
bool physical_device::is_other() const noexcept {
    return properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER;
}

const VkPhysicalDeviceFeatures& physical_device::features() const noexcept {
    return features_;
}

std::string_view physical_device::name() const noexcept {
    return properties_.deviceName;
}

physical_device::id_type physical_device::id() const noexcept {
    return properties_.deviceID;
}

VkPhysicalDeviceLimits physical_device::limits() const noexcept {
    return properties_.limits;
}

bool physical_device::has_geometry_shader() const noexcept {
    return features_.geometryShader;
}

} // namespace zoo::render::vulkan::utils
