#pragma once
#include <utility>

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2)      CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)
#endif

namespace zoo {
template <typename Fn>
struct Defer {
    Defer(Fn&& f) : fn(std::move(f)) {}
    ~Defer() { fn(); }

private:
    Fn fn;
};
} // namespace zoo

namespace zoo::detail {

enum class DeferHelper {};

template <typename Fn>
Defer<Fn> operator+(DeferHelper, Fn&& fn) {
    return Defer<Fn>{ std::forward<Fn&&>(fn) };
}

} // namespace zoo::detail

#define defer auto ANONYMOUS_VARIABLE(DEFER_FUNCTION) = ::zoo::detail::DeferHelper() + [&]()
#define DEFER defer


#define ZOO_ASSERT_COND(condition)                  assert(condition)
#define ZOO_ASSERT_COND_MESSAGE(condition, message) assert((condition) && message)

#define __GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define ZOO_ASSERT(...)                      __GET_3RD_ARG(__VA_ARGS__, ZOO_ASSERT_COND_MESSAGE, ZOO_ASSERT_COND)(__VA_ARGS__)

#define is_power_of_two(v) (((v) != 0) && (((v) & ((v)-1)) == 0))

