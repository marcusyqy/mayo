#pragma once
#include "render/vulkan/Device.hpp"
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan::utils {

struct type_map {};

// template<typename T>
// concept type = requires(T a) {
//     std::declval<vulkan::device>().release_device_resource(a);
//     { a != VK_NULL_HANDLE } -> std::convertible_to<bool>;
//     std::is_trivially_copyable_v<T>;
// };

template<typename T>
class box {
public:
    box() noexcept : device_(nullptr), type_(VK_NULL_HANDLE) {}
    box(std::shared_ptr<vulkan::device> device, T type)
        : device_(std::move(device)), type_(type) {}

    box(const box& other) noexcept = delete;
    box& operator=(const box& other) noexcept = delete;

    box(box&& other) noexcept : box() { this = std::move(other); }

    box& operator=(box&& other) noexcept {
        std::swap(device_, other.device_);
        std::swap(type_, other.type_);
    }

    ~box() noexcept { release(); }

    void release() noexcept {
        if (device_) {
            device_->release_device_resource(type_);
            type_ = VK_NULL_HANDLE;
        }
    }
    operator bool() const noexcept { return type_ != VK_NULL_HANDLE; }

    [[nodiscard]] operator T() const noexcept { return type_; }
    [[nodiscard]] T get() const noexcept { return type_; }

private:
    std::shared_ptr<vulkan::device> device_;
    T type_;
};

} // namespace zoo::render::vulkan::utils
