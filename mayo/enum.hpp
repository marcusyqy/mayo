#pragma once
#include <type_traits>

namespace mayo::detail {

template <typename Type, typename = void>
struct enum_as_bitmask : std::false_type {};

template <typename Type>
struct enum_as_bitmask<Type, std::void_t<decltype(Type::_mayo_enum_as_bitmask)>> : std::is_enum<Type> {};

template <typename Type>
constexpr auto enum_as_bitmask_v = enum_as_bitmask<Type>::value;

template <typename Type, typename = void>
struct enum_decay : std::false_type {};

template <typename Type>
struct enum_decay<Type, std::void_t<decltype(Type::_std_decay)>> : std::is_enum<Type> {};

template <typename Type>
constexpr auto enum_decay_v = enum_decay<Type>::value;

} // namespace mayo::detail

namespace mayo {

template <auto v>
struct enum_as_size {
    static constexpr size_t value = static_cast<size_t>(v);
};

template <auto v>
constexpr auto enum_as_size_v = enum_as_size<v>::value;
} // namespace mayo

template <typename Type>
[[nodiscard]] constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, Type>
    operator|(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(
        static_cast<std::underlying_type_t<Type>>(lhs) | static_cast<std::underlying_type_t<Type>>(rhs));
}

template <typename Type>
[[nodiscard]] constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, Type>
    operator&(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(
        static_cast<std::underlying_type_t<Type>>(lhs) & static_cast<std::underlying_type_t<Type>>(rhs));
}

template <typename Type>
[[nodiscard]] constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, Type>
    operator^(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(
        static_cast<std::underlying_type_t<Type>>(lhs) ^ static_cast<std::underlying_type_t<Type>>(rhs));
}

template <typename Type>
[[nodiscard]] constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, Type>
    operator~(const Type value) noexcept {
    return static_cast<Type>(~static_cast<std::underlying_type_t<Type>>(value));
}

template <typename Type>
[[nodiscard]] constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, bool>
    operator!(const Type value) noexcept {
    return !static_cast<std::underlying_type_t<Type>>(value);
}

template <typename Type>
constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, Type&>
    operator|=(Type& lhs, const Type rhs) noexcept {
    return (lhs = (lhs | rhs));
}

template <typename Type>
constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, Type&>
    operator&=(Type& lhs, const Type rhs) noexcept {
    return (lhs = (lhs & rhs));
}

template <typename Type>
constexpr std::enable_if_t<mayo::detail::enum_as_bitmask_v<Type>, Type&>
    operator^=(Type& lhs, const Type rhs) noexcept {
    return (lhs = (lhs ^ rhs));
}

template <typename Type>
constexpr std::enable_if_t<mayo::detail::enum_decay_v<Type>, std::underlying_type_t<Type>>
    operator+(const Type& t) noexcept {
    return static_cast<std::underlying_type_t<Type>>(t);
}
