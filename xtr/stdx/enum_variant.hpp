

#pragma once
#include <type_traits>

namespace stdx {

template <typename T, typename... Args>
struct enum_variant_impl {
    static_assert(std::is_integral_v<std::underlying_type_t<T>>, "Must be integral type");
};

} // namespace stdx
