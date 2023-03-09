#pragma once
#include "contiguous_iterator.hpp"
#include "fwd.hpp"
#include "type_traits.hpp"
#include <memory>

namespace stdx {

namespace detail {
template<typename T>
struct vector_traits {
    static constexpr bool is_const = std::is_const_v<T>;
    using size_type = size_t;
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;
    using pointer = std::conditional_t<is_const, const_pointer, value_type*>;
    using reference =
        std::conditional_t<is_const, const_reference, value_type&>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using index_type = size_type;
};
} // namespace detail

template<typename T, typename Allocator = std::allocator<T>>
class vector {
public:
    using size_type = typename detail::vector_traits<T>::size_type;
    using element_type = typename detail::vector_traits<T>::element_type;
    using value_type = typename detail::vector_traits<T>::value_type;
    using const_pointer = typename detail::vector_traits<T>::const_pointer;
    using const_reference = typename detail::vector_traits<T>::const_reference;
    using pointer = typename detail::vector_traits<T>::pointer;
    using reference = typename detail::vector_traits<T>::reference;
    using difference_type = typename detail::vector_traits<T>::difference_type;
    using iterator_category =
        typename detail::vector_traits<T>::iterator_category;
    using index_type = typename detail::vector_traits<T>::index_type;

    using const_iterator = contiguous_iterator<true, detail::vector_traits<T>>;
    using iterator = contiguous_iterator<false, detail::vector_traits<T>>;

private:
};

} // namespace stdx
