#pragma once
#include "render/device.hpp"

#include <cstddef>
#include <memory>
#include <stdx/type_traits.hpp>
#include <utility>
#include <vulkan/vulkan.h>

namespace zoo::render::utils {

struct type_map {};

namespace detail {

template<typename T>
using device_release_resource_exist_t =
    std::void_t<decltype(std::declval<device>().release_device_resource(
        std::declval<T>()))>;

} // namespace detail

template<typename T>
using is_valid_device_obj_t =
    std::void_t<detail::device_release_resource_exist_t<T>,
        stdx::nullptr_check_exists_t<T>,
        stdx::condition_type_t<std::is_assignable_v<T, std::nullptr_t>>,
        stdx::condition_type_t<std::is_trivially_copyable_v<T>>>;

template<typename T, typename = is_valid_device_obj_t<T>>
class box {
public:
    box() noexcept : device_(nullptr), type_(nullptr) {}
    box(std::shared_ptr<device> device, T type)
        : device_(std::move(device)), type_(type) {}

    box(const box& other) noexcept = delete;
    box& operator=(const box& other) noexcept = delete;

    box(box&& other) noexcept : box() { this = std::move(other); }

    box& operator=(box&& other) noexcept {
        std::swap(device_, other.device_);
        std::swap(type_, other.type_);
    }

    ~box() noexcept { reset(); }

    T release() noexcept {
        T ret = type_;
        type_ = nullptr;
        return ret;
    }

    void reset() noexcept {
        if (device_) {
            device_->release_device_resource(type_);
            type_ = nullptr;
        }
    }

    operator bool() const noexcept { return type_ != nullptr; }

    [[nodiscard]] operator T() const noexcept { return type_; }
    [[nodiscard]] T get() const noexcept { return type_; }

private:
    std::shared_ptr<device> device_;
    T type_;
};

} // namespace zoo::render::utils
