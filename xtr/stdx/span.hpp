#pragma once
#include "fwd.hpp"
#include "type_traits.hpp"
#include <array>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "contiguous_iterator.hpp"

namespace stdx {

constexpr auto dynamic_extent = std::numeric_limits<size_t>::max();

namespace detail {
template <typename T>
struct span_traits {
    static constexpr bool is_const = std::is_const_v<T>;
    using size_type                = size_t;
    using element_type             = T;
    using value_type               = std::remove_cv_t<T>;
    using const_pointer            = const value_type*;
    using const_reference          = const value_type&;
    using pointer                  = std::conditional_t<is_const, const_pointer, value_type*>;
    using reference                = std::conditional_t<is_const, const_reference, value_type&>;
    using difference_type          = std::ptrdiff_t;
    using iterator_category        = std::random_access_iterator_tag;
    using index_type               = size_type;
};
} // namespace detail

template <typename T, size_t N = stdx::dynamic_extent>
class span {
public:
    using size_type         = typename detail::span_traits<T>::size_type;
    using element_type      = typename detail::span_traits<T>::element_type;
    using value_type        = typename detail::span_traits<T>::value_type;
    using const_pointer     = typename detail::span_traits<T>::const_pointer;
    using const_reference   = typename detail::span_traits<T>::const_reference;
    using pointer           = typename detail::span_traits<T>::pointer;
    using reference         = typename detail::span_traits<T>::reference;
    using difference_type   = typename detail::span_traits<T>::difference_type;
    using iterator_category = typename detail::span_traits<T>::iterator_category;
    using index_type        = typename detail::span_traits<T>::index_type;
    using const_iterator    = contiguous_iterator<true, detail::span_traits<T>>;
    using iterator          = contiguous_iterator<false, detail::span_traits<T>>;

    iterator begin() noexcept { return { start_, 0 }; }
    iterator end() noexcept { return { start_, N }; }

    const_iterator begin() const noexcept { return { start_, 0 }; }
    const_iterator end() const noexcept { return { start_, N }; }

    const_iterator cbegin() const noexcept { return { start_, 0 }; }
    const_iterator cend() const noexcept { return { start_, N }; }

    size_type size() const noexcept { return N; }
    pointer data() const noexcept { return start_; }

    reference operator[](index_type idx) noexcept { return const_cast<reference>(std::as_const(*this)[idx]); }

    const_reference operator[](index_type idx) const noexcept {
        STDX_ASSERT(idx < N, "span operator[] index out of size");
        return start_[idx];
    }

    reference at(index_type idx) { return const_cast<reference>(std::as_const(*this).at(idx)); }

    const_reference at(index_type idx) const {
        if (idx < N) {
            return start_[idx];
        }
        // throwing out of range out of courtesy
        throw std::out_of_range("Out of range for span<T> `at` function");
    }

    span(pointer data, [[maybe_unused]] size_type size) noexcept : start_(data) {}

    ~span() noexcept = default;

    template <typename V, typename = stdx::is_container<V>>
    span(const V& container) : start_(container.data()) {
        STDX_ASSERT(container.size() >= N, "N cannot be greater than size");
    }

    template <typename V, typename = stdx::is_container<V>>
    span(V& container) : start_(container.data()) {
        STDX_ASSERT(container.size() >= N, "N cannot be greater than size");
    }

    template <typename TT, size_t NN>
    span(TT (&arr)[NN]) noexcept : span(arr, N) {
        static_assert(N <= NN, "Must be larger or equal to N");
    }

private:
    pointer start_;
};

template <typename T>
class span<T, stdx::dynamic_extent> {
public:
    using size_type         = typename detail::span_traits<T>::size_type;
    using element_type      = typename detail::span_traits<T>::element_type;
    using value_type        = typename detail::span_traits<T>::value_type;
    using const_pointer     = typename detail::span_traits<T>::const_pointer;
    using const_reference   = typename detail::span_traits<T>::const_reference;
    using pointer           = typename detail::span_traits<T>::pointer;
    using reference         = typename detail::span_traits<T>::reference;
    using difference_type   = typename detail::span_traits<T>::difference_type;
    using iterator_category = typename detail::span_traits<T>::iterator_category;
    using index_type        = typename detail::span_traits<T>::index_type;
    using const_iterator    = contiguous_iterator<true, detail::span_traits<T>>;
    using iterator          = contiguous_iterator<false, detail::span_traits<T>>;

    iterator begin() noexcept { return { start_, 0 }; }
    iterator end() noexcept { return { start_, size_ }; }

    const_iterator begin() const noexcept { return { start_, 0 }; }
    const_iterator end() const noexcept { return { start_, size_ }; }

    const_iterator cbegin() const noexcept { return { start_, 0 }; }
    const_iterator cend() const noexcept { return { start_, size_ }; }

    size_type size() const noexcept { return size_; }
    pointer data() const noexcept { return start_; }

    reference operator[](index_type idx) noexcept { return const_cast<reference>(std::as_const(*this)[idx]); }

    const_reference operator[](index_type idx) const noexcept {
        STDX_ASSERT(idx < size_, "span operator[] index out of size");
        return start_[idx];
    }

    reference at(index_type idx) { return const_cast<reference>(std::as_const(*this).at(idx)); }

    const_reference at(index_type idx) const {
        if (idx < size_) {
            return start_[idx];
        }
        // throwing out of range out of courtesy
        throw std::out_of_range("Out of range for span<T> `at` function");
    }

    span() noexcept : start_(nullptr), size_(0) {}
    span(pointer data, size_type size) noexcept : start_(data), size_(size) {}
    ~span() noexcept = default;

    template <typename V, typename = stdx::is_container<V>>
    span(const V& container) noexcept : start_(container.data()), size_(container.size()) {}

    template <typename V, typename = stdx::is_container<V>>
    span(V& container) noexcept : start_(container.data()), size_(container.size()) {}

    template <typename TT, size_t N>
    span(TT (&arr)[N]) noexcept : span(arr, N) {}

    // template <typename TT>
    // span(TT* data) noexcept : span(data, data != nullptr ? 1 : 0) {}
    span(std::nullptr_t) noexcept : span(nullptr, 0) {}

private:
    pointer start_;
    size_type size_;
};

template <typename TT, size_t N>
span(TT (&arr)[N]) -> span<TT>;

template <typename TT, size_t N>
span(const TT (&arr)[N]) -> span<const TT>;

template <typename V, typename = stdx::is_contiguous_container<V>>
span(V&) -> span<typename V::value_type>;

template <typename V, typename = stdx::is_contiguous_container<V>>
span(const V&) -> span<const typename V::value_type>;

template <typename V>
span(V*) -> span<V>;

template <typename V>
span(const V*) -> span<const V>;

} // namespace stdx
