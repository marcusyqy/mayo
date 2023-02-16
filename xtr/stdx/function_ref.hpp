#pragma once

#include <type_traits>
#include <utility>

namespace stdx {

template<typename>
class function_ref;

template<typename R, typename... Args>
class function_ref<R(Args...) noexcept> {
public:
    template<typename Callable,
        bool = std::is_invocable_r_v<R, Callable&&, Args...>>
    function_ref(Callable&& c) noexcept
        : obj_(std::addressof(c)), cb_([](void* obj, Args... args) {
              return std::invoke(
                  *static_cast<std::add_pointer_t<Callable&&>>(obj),
                  std::forward<Args>(args)...);
          }) {}

    template<typename... PArgs>
    decltype(auto) operator()(PArgs&&... args) {
        return cb_(obj_, std::forward<PArgs>(args)...);
    }

private:
    void* obj_ = nullptr;
    R (*cb_)(void*, Args...) = nullptr;
};

template<typename R, typename... Args>
class function_ref<R(Args...)> {
public:
    template<typename Callable,
        bool = std::is_invocable_r_v<R, Callable&&, Args...>>
    function_ref(Callable&& c) noexcept
        : obj_(std::addressof(c)), cb_([](void* obj, Args... args) {
              return std::invoke(
                  *static_cast<std::add_pointer_t<Callable&&>>(obj),
                  std::forward<Args>(args)...);
          }) {}

    template<typename... PArgs>
    decltype(auto) operator()(PArgs&&... args) {
        return cb_(obj_, std::forward<PArgs>(args)...);
    }

private:
    void* obj_ = nullptr;
    R (*cb_)(void*, Args...) = nullptr;
};

} // namespace stdx
