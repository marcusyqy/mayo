#pragma once
#include <type_traits>
#include <utility>

namespace stdx {
template <typename Type>
struct type_identity {
    using type = Type;
};

// type identity for disambiguiting args
template <typename Type>
using type_identity_t = typename type_identity<Type>::type;

template <bool = false>
struct condition_type;

template <>
struct condition_type<true> {
    using type = int;
};

template <bool expr>
using condition_type_t = typename condition_type<expr>::type;

// checks whether this type has a null check. can be any type as long as the
// null check is constexpr
template <typename T, auto null_type = nullptr>
struct null_value_check_exists : std::is_same<std::decay_t<decltype(std::declval<T>() != null_type)>, bool> {};

template <typename T, auto null_type = nullptr>
constexpr auto null_value_check_exists_v = null_value_check_exists<T, null_type>::value;

template <typename T, typename null_type = std::nullptr_t>
struct null_type_check_exists :
    std::is_same<std::decay_t<decltype(std::declval<T>() != std::declval<null_type>())>, bool> {};

template <typename T, typename null_type = std::nullptr_t>
constexpr auto null_type_check_exists_v = null_type_check_exists<T, null_type>::value;

template <typename T>
using nullptr_check_exists_t = condition_type_t<null_value_check_exists_v<T, nullptr>>;

template <typename T>
using is_container = std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>;

template <typename T>
using is_contiguous_container = std::void_t<
    typename T::value_type,
    std::enable_if<std::is_same_v<decltype(std::declval<T>().data()), typename T::pointer>, bool>,
    std::enable_if<std::is_same_v<decltype(std::as_const(std::declval<T>()).data()), typename T::const_pointer>, bool>,
    std::enable_if<std::is_same_v<decltype(std::declval<T>().size()), typename T::size_type>, bool>,
    stdx::is_container<T>>;

} // namespace stdx
