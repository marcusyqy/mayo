#pragma once

#include "render/device_context.hpp"
#include "render/fwd.hpp"
#include "stdx/type_traits.hpp"
#include <memory>

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
using is_valid_device_obj_t = device_context_release_resource_exist_t<T>;

template<typename T, typename = is_valid_device_obj_t<T>>
class box {
public:
    using value_type = T;

    box() noexcept : context_(nullptr), type_(nullptr) {}
    box(std::shared_ptr<device_context> device, T type)
        : context_(std::move(device)), type_(type) {}

    box(const box& other) noexcept = delete;
    box& operator=(const box& other) noexcept = delete;

    box(box&& other) noexcept : box() { *this = std::move(other); }

    box& operator=(box&& other) noexcept {
        std::swap(context_, other.context_);
        std::swap(type_, other.type_);
        return *this;
    }

    void set(std::shared_ptr<device_context> device, T type) {
        context_ = std::move(device);
        type_ = type;
    }

    ~box() noexcept { reset(); }

    value_type release() noexcept {
        value_type ret = type_;
        type_ = nullptr;
        return ret;
    }

    void reset() noexcept {
        if (context_) {
            context_->release_device_resource(release());
        }
    }

    operator bool() const noexcept { return type_ != nullptr; }

    [[nodiscard]] operator value_type() const noexcept { return type_; }
    [[nodiscard]] value_type get() const noexcept { return type_; }

private:
    std::shared_ptr<device_context> context_;
    T type_;
};

} // namespace zoo::render::utils
