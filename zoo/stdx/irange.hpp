
#pragma once
#include "stdx/function_ref.hpp"
#include "type_traits.hpp"
#include <type_traits>

namespace stdx {

template<typename I, std::enable_if_t<std::is_integral_v<I>, bool> = true>
class integer_range {
public:
    struct iterator {
        iterator(I value) noexcept : value_(value) {}

        iterator& operator++() noexcept {
            ++value_;
            return *this;
        }
        iterator operator++(int) noexcept { return value_++; }
        iterator& operator--() noexcept {
            --value_;
            return *this;
        }
        iterator operator--(int) noexcept { return value_--; }

        I operator*() const noexcept { return value_; }

        // comparison
        bool operator==(const iterator& e) const noexcept {
            return value_ == e.value_;
        }

        bool operator!=(const iterator& e) const noexcept {
            return value_ != e.value_;
        }

        bool operator<(const iterator& e) const noexcept {
            return value_ < e.value_;
        }

        bool operator>(const iterator& e) const noexcept {
            return value_ > e.value_;
        }
        bool operator<=(const iterator& e) const noexcept {
            return value_ <= e.value_;
        }

        bool operator>=(const iterator& e) const noexcept {
            return value_ >= e.value_;
        }

    private:
        I value_;
    };
    using const_iterator = iterator;

    const_iterator begin() const noexcept { return begin_; }
    const_iterator end() const noexcept { return end_; }

    void for_each(stdx::function_ref<void(I)> f) noexcept {
        for (auto i : *this) {
            f(i);
        }
    }

    integer_range(I begin, I end) : begin_(begin), end_(end) {}

private:
    I begin_;
    I end_;
};

template<typename I>
integer_range<I> irange(stdx::type_identity_t<I> begin, I end) noexcept {
    return {begin, end};
}

} // namespace stdx
