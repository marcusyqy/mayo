#pragma once

#include "render/device_context.hpp"
#include "stdx/type_traits.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace zoo::render::utils {

template<typename T>
struct device_context_release_resource_exist {
    using type =
        std::void_t<decltype(std::declval<device_context>()
                                 .release_device_resource(std::declval<T>()))>;
};

template<typename T>
using device_context_release_resource_exist_t =
    typename device_context_release_resource_exist<T>::type;

template<typename T>
using is_valid_device_obj_t =
    std::void_t<device_context_release_resource_exist_t<T>,
        stdx::nullptr_check_exists_t<T>,
        stdx::condition_type_t<std::is_assignable_v<T, std::nullptr_t>>,
        stdx::condition_type_t<std::is_trivially_copyable_v<T>>>;

template<typename T, typename = is_valid_device_obj_t<T>>
class box {
public:
    box() noexcept : context_(nullptr), type_(nullptr) {}
    box(std::shared_ptr<device_context> device, T type) :
        context_(std::move(device)), type_(type) {}

    box(const box& other) noexcept = delete;
    box& operator=(const box& other) noexcept = delete;

    box(box&& other) noexcept : box() { this = std::move(other); }

    box& operator=(box&& other) noexcept {
        std::swap(context_, other.context_);
        std::swap(type_, other.type_);
    }

    ~box() noexcept { reset(); }

    T release() noexcept {
        T ret = type_;
        type_ = nullptr;
        return ret;
    }

    void reset() noexcept {
        if (context_) {
            context_->release_device_resource(type_);
            type_ = nullptr;
        }
    }

    operator bool() const noexcept { return type_ != nullptr; }

    [[nodiscard]] operator T() const noexcept { return type_; }
    [[nodiscard]] T get() const noexcept { return type_; }

private:
    std::shared_ptr<device_context> context_;
    T type_;
};

} // namespace zoo::render::utils
