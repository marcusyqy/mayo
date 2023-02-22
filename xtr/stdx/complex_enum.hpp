#pragma once
#include "variant.hpp"

namespace stdx {

namespace detail {

template<auto X, typename T>
struct complex_enum_pair {};

template<typename T>
struct complex_enum_get_value;

template<auto X, typename T>
struct complex_enum_get_value<complex_enum_pair<X, T>> {
    using type = T;
};

template<typename T>
using complex_enum_get_value_t = typename complex_enum_get_value<T>::type;

} // namespace detail

template<typename T, typename... Args>
class complex_enum {
public:
    template<auto X, typename V>
    using pair = detail::complex_enum_pair<X, V>;

private:
    T type_;
    variant<detail::complex_enum_get_value_t<Args>...> value_;
};
} // namespace stdx
