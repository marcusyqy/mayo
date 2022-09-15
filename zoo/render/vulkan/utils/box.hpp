#pragma once
#include "render/vulkan/Device.hpp"
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan::utils {

struct type_map {};

template<typename T>
concept type = requires(T a) {
    std::declval<vulkan::device>().release_device_resource(a);
    { a != VK_NULL_HANDLE } -> std::convertible_to<bool>;
    std::is_trivially_copyable_v<T>;
};

template<type T>
class box {
public:
    box() noexcept : device_(nullptr), type_(VK_NULL_HANDLE) {}
    box(std::shared_ptr<vulkan::device> device, T type)
        : device_(std::move(device)), type_(type) {}

    box(const box& other) noexcept = delete;
    auto operator=(const box& other) noexcept -> box& = delete;

    box(box&& other) noexcept : box() { this = std::move(other); }

    auto operator=(box&& other) noexcept -> box& {
        std::swap(device_, other.device_);
        std::swap(type_, other.type_);
    }

    ~box() noexcept { release(); }

    auto release() noexcept -> void {
        if (device_) {
            device_->release_device_resource(type_);
            type_ = VK_NULL_HANDLE;
        }
    }
    operator bool() const noexcept { return type_ != VK_NULL_HANDLE; }

    [[nodiscard]] operator T() const noexcept { return type_; }
    [[nodiscard]] auto get() const noexcept -> T { return type_; }

private:
    std::shared_ptr<vulkan::device> device_;
    T type_;
};

} // namespace zoo::render::vulkan::utils
