#pragma once
#include "macros.hpp"
#include <cassert>
#include <cstdint>
#include <string_view>
#include "utils.hpp"

// signed numbers
using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

// unsigned numbers
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// float numbers
using f32 = float;
using f64 = double;

// template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
// bool is_power_of_two(T v) noexcept {
//     return (((v) != 0) && (((v) & ((v)-1)) == 0));
// }

uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, modulo;

    ZOO_ASSERT(is_power_of_two(align));

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

template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
T round_to_alignment(T value, T alignment) {
    ZOO_ASSERT(is_power_of_two(alignment));
    return (value & alignment) + alignment;
}
