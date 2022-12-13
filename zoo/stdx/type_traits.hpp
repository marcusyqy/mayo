#pragma once

namespace stdx {
template<typename Type>
struct type_identity {
    using type = Type;
};

// type identity for disambiguiting args
template<typename Type>
using type_identity_t = typename type_identity<Type>::type;

} // namespace stdx