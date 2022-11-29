#pragma once
#include <type_traits>

namespace stdx::detail {

template<typename Type, typename = void>
struct enum_as_bitmask : std::false_type {};

template<typename Type>
struct enum_as_bitmask<Type, std::void_t<decltype(Type::_stdx_enum_as_bitmask)>>
    : std::is_enum<Type> {};

template<typename Type>
constexpr auto enum_as_bitmask_v = enum_as_bitmask<Type>::value;
} // namespace stdx::detail

template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>,
    Type>
operator|(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(lhs) |
                             static_cast<std::underlying_type_t<Type>>(rhs));
}

template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>,
    Type>
operator&(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(lhs) &
                             static_cast<std::underlying_type_t<Type>>(rhs));
}

template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>,
    Type>
operator^(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(lhs) ^
                             static_cast<std::underlying_type_t<Type>>(rhs));
}

template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>,
    Type>
operator~(const Type value) noexcept {
    return static_cast<Type>(~static_cast<std::underlying_type_t<Type>>(value));
}

template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>,
    bool>
operator!(const Type value) noexcept {
    return !static_cast<std::underlying_type_t<Type>>(value);
}

template<typename Type>
constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>, Type&>
operator|=(Type& lhs, const Type rhs) noexcept {
    return (lhs = (lhs | rhs));
}

template<typename Type>
constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>, Type&>
operator&=(Type& lhs, const Type rhs) noexcept {
    return (lhs = (lhs & rhs));
}

template<typename Type>
constexpr std::enable_if_t<stdx::detail::enum_as_bitmask_v<Type>, Type&>
operator^=(Type& lhs, const Type rhs) noexcept {
    return (lhs = (lhs ^ rhs));
}
