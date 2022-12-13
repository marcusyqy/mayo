#pragma once
#include <memory>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan {

class physical_device {
public:
    using underlying_type = VkPhysicalDevice;

    operator underlying_type() const noexcept { return underlying_; }
    physical_device(underlying_type underlying) noexcept;

    // device type
    [[nodiscard]] bool is_discrete() const noexcept;
    [[nodiscard]] bool is_integrated() const noexcept;
    [[nodiscard]] bool is_virtual() const noexcept;
    [[nodiscard]] bool is_cpu() const noexcept;
    [[nodiscard]] bool is_other() const noexcept;

private:
    void query_properties_and_features() noexcept;

private:
    VkPhysicalDevice underlying_ = nullptr;

    VkPhysicalDeviceProperties properties_{};
    VkPhysicalDeviceFeatures features_{};
};

class device : std::enable_shared_from_this<device> {
public:
    device(VkInstance instance, physical_device pdevice) noexcept;
    ~device() noexcept;

    device(const device& other) noexcept = delete;
    device(device&& other) noexcept = delete;

    void operator=(const device& other) noexcept = delete;
    void operator=(device&& other) noexcept = delete;
    void reset() noexcept;

    operator physical_device() const noexcept { return physical_; }

    void release_device_resource(VkFence fence) noexcept;

private:
    VkPhysicalDevice physical_ = nullptr;
    VkDevice logical_ = nullptr;
};

} // namespace zoo::render::vulkan
