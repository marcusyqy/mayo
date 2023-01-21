
#pragma once
#include <type_traits>

namespace stdx {

template<typename I, std::enable_if<std::is_integral_v<I>, bool> = true>
class irange {
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

    private:
        I value_;
    };
    using const_iterator = iterator;

    const_iterator begin() const noexcept { return begin_; }
    const_iterator end() const noexcept { return end_; }

private:
    I begin_;
    I end_;
};

template<typename I>
irange(I, I) -> irange<I>;

} // namespace stdx
