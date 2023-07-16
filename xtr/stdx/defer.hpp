#pragma once

#include <utility>

namespace stdx {

template<typename CB>
struct defer {
    defer(CB&& cb) : callback(std::move(cb)) {}
    ~defer() noexcept { callback(); }

private:
    CB callback;
};

template<typename T>
defer(T t) -> defer<T>;

} // namespace stdx

// this looks like it works. Revisit if defining multiple causes problems.
#define STDX_DEFER_CONCAT(a, b)          STDX_DEFER_CONCAT_INNER(a, b)
#define STDX_DEFER_CONCAT_INNER(a, b)    a##b
#define STDX_DEFER(x) [[maybe_unused]]   ::stdx::defer STDX_DEFER_CONCAT(_defer, __LINE__)(x)

