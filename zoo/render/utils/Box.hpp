#pragma once

#include "render/DeviceContext.hpp"
#include "render/fwd.hpp"
#include "stdx/type_traits.hpp"
#include <memory>

namespace zoo::render::utils {

template<typename T>
struct DeviceContextReleaseResourceExists {
    using type =
        std::void_t<decltype(std::declval<DeviceContext>()
                                 .release_device_resource(std::declval<T>()))>;
};

template<typename T>
using DeviceContextReleaseResourceExists_t =
    typename DeviceContextReleaseResourceExists<T>::type;

template<typename T>
using IsValidVulkanObj_t = DeviceContextReleaseResourceExists_t<T>;

template<typename T, typename = IsValidVulkanObj_t<T>>
class Box {
public:
    using value_type = T;

    Box() noexcept : context_(nullptr), type_(nullptr) {}
    Box(DeviceContext& device, T type)
        : context_(std::addressof(device)), type_(type) {}

    Box(const Box& other) noexcept = delete;
    Box& operator=(const Box& other) noexcept = delete;

    Box(Box&& other) noexcept : Box() { *this = std::move(other); }

    Box& operator=(Box&& other) noexcept {
        std::swap(context_, other.context_);
        std::swap(type_, other.type_);
        other.reset();
        return *this;
    }

    void emplace(DeviceContext& device, T type) {
        context_ = std::addressof(device);
        type_ = type;
    }

    ~Box() noexcept { reset(); }

    value_type release() noexcept {
        value_type ret = type_;
        type_ = nullptr;
        return ret;
    }

    void reset() noexcept {
        if (context_ != nullptr) {
            context_->release_device_resource(release());
            context_ = nullptr;
        }
    }

    operator bool() const noexcept { return type_ != nullptr; }

    [[nodiscard]] operator value_type() const noexcept { return type_; }
    [[nodiscard]] value_type get() const noexcept { return type_; }

protected:
    DeviceContext* context_;
    T type_;
};

} // namespace zoo::render::utils
