
#pragma once
#include "fwd.hpp"
#include "type_traits.hpp"

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

public:
    contiguous_iterator(pointer data, index_type idx) noexcept
        : data_(data), curr_(idx) {}

    this_type& operator++() noexcept { return ++curr_, *this; }
    this_type operator++(int) noexcept { return {data_, curr_++}; }

    // should probably check for 0
    this_type& operator--() noexcept { return --curr_, *this; }
    this_type operator--(int) noexcept { return {data_, curr_--}; }

    reference operator*() noexcept { return data_[curr_]; }
    const_reference operator*() const noexcept { return data_[curr_]; }

    bool operator==(const this_type& other) const noexcept {
        STDX_ASSERT(data_ == other.data_,
            "Not even comparing iterators from the same container!");
        return curr_ == other.curr_;
    }

    bool operator!=(const this_type& other) const noexcept {
        return !(*this == other);
    }

    bool operator<(const this_type& other) const noexcept {
        STDX_ASSERT(data_ == other.data_,
            "Not even comparing iterators from the same container!");
        return curr_ < other.curr_;
    }

    bool operator>(const this_type& other) const noexcept {
        STDX_ASSERT(data_ == other.data_,
            "Not even comparing iterators from the same container!");
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
}
