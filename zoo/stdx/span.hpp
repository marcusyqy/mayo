#pragma once
#include "fwd.hpp"
#include "type_traits.hpp"
#include <array>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace stdx {

template<bool Const, typename Owner>
struct contiguous_iterator {
    using owner_type = Owner;
    using this_type = contiguous_iterator<Const, owner_type>;
    using other_this_type = contiguous_iterator<!Const, owner_type>;
    using value_type = typename owner_type::value_type;
    using iterator_category = typename owner_type::iterator_category;
    using difference_type = typename owner_type::difference_type;
    using const_reference = typename owner_type::const_reference;
    using const_pointer = typename owner_type::const_pointer;
    using size_type = typename owner_type::size_type;
    using index_type = size_type;
    using reference = std::conditional_t<Const,
        typename owner_type::const_reference, typename owner_type::reference>;
    using pointer = std::conditional_t<Const,
        typename owner_type::const_pointer, typename owner_type::pointer>;

private:
    friend owner_type;
    contiguous_iterator(pointer data, index_type idx) noexcept :
        data_(data), curr_(idx) {}

public:
    this_type operator++() noexcept { return ++curr_, *this; }
    this_type operator++(int) noexcept { return {data_, curr_++}; }

    // should probably check for 0
    this_type operator--() noexcept { return --curr_, *this; }
    this_type operator--(int) noexcept { return {data_, curr_--}; }

    reference operator*() noexcept { return data_[curr_]; }
    const_reference operator*() const noexcept { return data_[curr_]; }

    bool operator==(const this_type& other) const noexcept {
        STDX_ASSERT(data_ == other.data_,
            "Not even comparing iterators from the same span!");
        return curr_ == other.curr_;
    }

    bool operator!=(const this_type& other) const noexcept {
        return !(*this == other);
    }

    bool operator<(const this_type& other) const noexcept {
        STDX_ASSERT(data_ == other.data_,
            "Not even comparing iterators from the same span!");
        return curr_ < other.curr_;
    }

    bool operator>(const this_type& other) const noexcept {
        STDX_ASSERT(data_ == other.data_,
            "Not even comparing iterators from the same span!");
        return curr_ > other.curr_;
    }

    bool operator<=(const this_type& other) const noexcept {
        return !(*this > other);
    }

    bool operator>=(const this_type& other) const noexcept {
        return !(*this < other);
    }

private:
    pointer data_;
    index_type curr_;
};

template<typename T>
class span final {
public:
    static constexpr bool is_const = std::is_const_v<T>;
    using size_type = size_t;
    using value_type = std::remove_const_t<T>;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;
    using pointer = std::conditional_t<is_const, const_pointer, value_type*>;
    using reference =
        std::conditional_t<is_const, const_reference, value_type&>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using index_type = size_type;

    using iterator = contiguous_iterator<false, span<T>>;
    using const_iterator = contiguous_iterator<true, span<T>>;

    iterator begin() noexcept { return {start_, 0}; }
    iterator end() noexcept { return {start_, size_}; }

    const_iterator begin() const noexcept { return {start_, 0}; }
    const_iterator end() const noexcept { return {start_, size_}; }

    const_iterator cbegin() const noexcept { return {start_, 0}; }
    const_iterator cend() const noexcept { return {start_, size_}; }

    size_type size() const noexcept { return size_; }
    pointer data() const noexcept { return start_; }

    reference operator[](index_type idx) noexcept {
        return const_cast<reference>(std::as_const(*this)[idx]);
    }

    const_reference operator[](index_type idx) const noexcept {
        STDX_ASSERT(idx < size_, "span operator[] index out of size");
        return start_[idx];
    }

    reference at(index_type idx) {
        return const_cast<reference>(std::as_const(*this).at(idx));
    }

    const_reference at(index_type idx) const {
        if (idx < size_) {
            return start_[idx];
        }
        // throwing out of range out of courtesy
        throw std::out_of_range("Out of range for span<T> `at` function");
    }

    span(pointer data, size_type size) noexcept : start_(data), size_(size) {}
    ~span() noexcept = default;

    template<typename V, typename = stdx::is_container_t<V>>
    span(const V& container) :
        start_(container.data()), size_(container.size()) {}

    template<typename V, typename = stdx::is_container_t<V>>
    span(V& container) : start_(container.data()), size_(container.size()) {}

private:
    pointer start_;
    size_type size_;
};

template<typename V, typename = stdx::is_contiguous_container_t<V>>
span(V&) -> span<typename V::value_type>;

template<typename V, typename = stdx::is_contiguous_container_t<V>>
span(const V&) -> span<const typename V::value_type>;

} // namespace stdx
