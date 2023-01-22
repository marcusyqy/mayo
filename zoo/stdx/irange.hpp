
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
        std::for_each(std::begin(*this), std::end(*this), f);
    }

    integer_range(I begin, I end) : begin_(begin), end_(end) {}

private:
    I begin_;
    I end_;
};

template<typename I>
integer_range<I> irange(I begin, I end) noexcept {
    return {begin, end};
}

// TODO: right now we're just assigning the bigger size number into I for
// integer_range but we should also compare signedness and unsignedness
template<typename I1, typename I2>
decltype(auto) irange(I1 begin, I2 end) noexcept {
    using I = std::conditional_t<sizeof(I1) < sizeof(I2), I2, I1>;
    // surpress warnings;
    return irange(static_cast<I>(begin), static_cast<I>(end));
}

} // namespace stdx
