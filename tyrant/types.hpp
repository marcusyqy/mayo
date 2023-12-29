#pragma once
#include <cstdint>
#include <cassert>

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

template <typename T>
struct Buffer_View {
    T* data      = { nullptr };
    size_t count = { 0 };

    const T& operator[](size_t idx) const noexcept {
        assert(idx < count);
        return data[idx];
    }

    T& operator[](size_t idx) noexcept {
        assert(idx < count);
        return data[idx];
    }
};

using String_View = Buffer_View<char>;

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))
