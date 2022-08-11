#pragma once

#include <compare>
#include <cstdint>
#include <string_view>

namespace zoo {
/*
 * this class is mainly for being able to write
 * hello<"hello"> in template arguments
 * */
template<unsigned N>
struct fixed_string {
    char buf[N + 1]{};

    constexpr fixed_string(char const* s) {
        for (size_t i{}; i != N; ++i)
            buf[i] = s[i];
    }

    constexpr operator char const *() const { return buf; }

    auto operator<=>(const fixed_string&) const = default;
};

template<unsigned N>
fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

} // namespace lzh
