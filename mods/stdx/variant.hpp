#pragma once

#include <memory>
#include <utility>
namespace stdx {

namespace detail {

template <typename... All>
struct variant_leaf {};

template <typename First, typename... Others>
struct variant_leaf<First, Others...> {

    variant_leaf() : value_{} {}

    template <size_t Size, typename T, typename... Args>
    constexpr size_t emplace(Args&&... args) noexcept {
        if constexpr (std::is_same_v<std::decay_t<T>, std::decay_t<First>>) {
            new T(std::addressof(value_.f), std::forward<Args&&>(args)...);
            return Size;
        } else {
            return value_.o.template emplace<Size + 1, T>(std::forward<Args&&>(args)...);
        }
    }

    template <size_t Size>
    decltype(auto) get() noexcept {
        if constexpr (Size == 0) {
            return value_.f;
        } else {
            return value_.o.template get<Size - 1>();
        }
    }

    template <typename Callable>
    decltype(auto) visit(Callable&& c, size_t idx) noexcept {
        if (idx == 0) {
            return c(value_.f);
        } else {
            return value_.o.visit(std::forward<Callable&&>(c), idx - 1);
        }
    }

    void destruct(size_t idx) noexcept {
        if (idx != 0) {
            destruct(idx - 1);
        } else {
            std::destroy_at(std::addressof(value_.f));
        }
    }

    union {
        First f;
        variant_leaf<Others...> o;
    } value_;
};

} // namespace detail
  //
  //

struct monostate {};

// should this type pack be unique?
template <typename... Ts>
class variant {
public:
    template <typename T, typename... Args>
    void emplace(Args&&... args) noexcept {
        idx_ = args_.template emplace<0, T>(std::forward<Args&&>(args)...);
    }

    template <size_t Size>
    decltype(auto) get() noexcept {
        return args_.template get<Size>();
    }

    template <typename Callable>
    decltype(auto) visit(Callable&& c) noexcept {
        return args_.visit(std::forward<Callable&&>(c), idx_);
    }

    ~variant() noexcept { args_.destruct(idx_); }

    template <typename T>
    variant(T&& arg) noexcept : idx_(0), args_{} {
        emplace<std::decay_t<T>>(std::forward<T&&>(arg));
    }

    variant() noexcept : idx_(0), args_{ 0 } {}

private:
    size_t idx_                       = 0;
    detail::variant_leaf<Ts...> args_ = {};
};

} // namespace stdx
