#pragma once
#include "zoo.hpp"

#include "utils/physical_device.hpp"

#include "core/platform/query.hpp"
#include "fwd.hpp"
#include <memory>

namespace zoo::render {

class device_context : enable_ref_from_this<device_context> {
public:
    // traits (?)

    device_context(VkInstance instance, utils::physical_device pdevice,
        const utils::queue_family_properties& family_props,
        const platform::render::query& query) noexcept;

    ~device_context() noexcept;

    device_context(const device_context& other) noexcept = delete;
    device_context(device_context&& other) noexcept = delete;

    device_context& operator=(const device_context& other) noexcept = delete;
    device_context& operator=(device_context&& other) noexcept = delete;

    void reset() noexcept;

    operator const utils::physical_device&() const noexcept {
        return physical();
    }
    const utils::physical_device& physical() const noexcept {
        return physical_;
    }

    operator const VkDevice&() const noexcept { return logical(); }
    const VkDevice& logical() const noexcept { return logical_; }

    VkCommandBuffer buffer_from_pool() const noexcept;

    VkQueue retrieve(operation op) const noexcept;

    // release resource
    void release_device_resource(VkFence fence) noexcept;
    void release_device_resource(VkRenderPass renderpass) noexcept;
    void release_device_resource(VkSemaphore semaphore) noexcept;
    void release_device_resource(VkBuffer buffer) noexcept;
    void release_device_resource(VkDeviceMemory device_memory) noexcept;

    void wait() noexcept;

private:
    utils::physical_device physical_ = nullptr;
    VkDevice logical_ = nullptr;
    VkQueue queue_ = nullptr;
    utils::queue_family_properties queue_properties_;
    VkCommandPool command_pool_ = nullptr;
};

} // namespace zoo::render
