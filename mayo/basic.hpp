#pragma once
#include <cassert>
#include <cstdint>
#include <utility>

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2)      CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)
#endif

// should try to use namespaces? or no
namespace detail {
template <typename Fn>
struct Defer {
    Defer(Fn&& f) : fn(std::move(f)) {}
    ~Defer() { fn(); }

private:
    Fn fn;
};

enum class DeferHelper {};

template <typename Fn>
Defer<Fn> operator+(DeferHelper, Fn&& fn) {
    return Defer<Fn>{ std::forward<Fn&&>(fn) };
}

} // namespace detail

#define defer              auto ANONYMOUS_VARIABLE(DEFER_FUNCTION) = ::detail::DeferHelper() + [&]()
#define DEFER              defer
#define is_power_of_two(v) (((v) != 0) && (((v) & ((v)-1)) == 0))

#define round_to_alignment(value, alignment)                                                                           \
    {                                                                                                                  \
        assert(is_power_of_two(alignment));                                                                            \
        return (value & alignment) + alignment;                                                                        \
    }

namespace convert_to {
constexpr size_t kilo_bytes(size_t bytes) noexcept { return ((size_t)bytes << 10); }
constexpr size_t mega_bytes(size_t bytes) noexcept { return ((size_t)bytes << 20); }
constexpr size_t giga_bytes(size_t bytes) noexcept { return ((size_t)bytes << 30); }
} // namespace convert_to

constexpr size_t DEFAULT_ALIGNMENT = 2 * sizeof(void*);

static uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, modulo;
    assert(is_power_of_two(align));

    p = ptr;
    a = (uintptr_t)align;
    // Same as (p % a) but faster as 'a' is a power of two
    modulo = p & (a - 1);

    if (modulo != 0) {
        // If 'p' address is not aligned, push the address to the
        // next value which is aligned
        p += a - modulo;
    }
    return p;
}

