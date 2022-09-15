#pragma once
#include "render/vulkan/Device.hpp"
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>
#include <vulkan/vulkan.h>

namespace zoo::render::vulkan::utils {

struct TypeMap {};

template<typename T>
concept Type = requires(T a) {
    std::declval<vulkan::Device>().release_device_resource(a);
    { a != VK_NULL_HANDLE } -> std::convertible_to<bool>;
    std::is_trivially_copyable_v<T>;
};

template<Type T>
class Box {
public:
    Box() noexcept : device_(nullptr), type_(VK_NULL_HANDLE) {}
    Box(std::shared_ptr<vulkan::Device> device, T type)
        : device_(std::move(device)), type_(type) {}

    Box(const Box& other) noexcept = delete;
    auto operator=(const Box& other) noexcept -> Box& = delete;

    Box(Box&& other) noexcept : Box() { this = std::move(other); }

    auto operator=(Box&& other) noexcept -> Box& {
        std::swap(device_, other.device_);
        std::swap(type_, other.type_);
    }

    ~Box() noexcept { release(); }

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
    std::shared_ptr<vulkan::Device> device_;
    T type_;
};

} // namespace zoo::render::vulkan::utils