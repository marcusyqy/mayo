#pragma once

#include <string>
#include <variant>

namespace stdx {

namespace detail::error {

struct error_t {
    std::string error_msg;
};
} // namespace detail::error

template<typename Error>
class unexpected {
public:
    using error_type = Error;

    // accessors
    constexpr const error_type& error() const& noexcept { return error_; }
    constexpr error_type& error() & noexcept { return error_; }
    constexpr const error_type&& error() const&& noexcept {
        return std::move(error_);
    }
    constexpr error_type&& error() && noexcept { return std::move(error_); }

    unexpected& operator=(const error_type& error) noexcept {
        error_ = error;
        return *this;
    }

    unexpected& operator=(error_type&& error) noexcept {
        std::swap(error_, error);
        return *this;
    }

private:
    error_type error_;
};

template<typename ValueType, typename Error = detail::error::error_t>
class expected {
public:
    using value_type = ValueType;
    using error_type = Error;
    using unexpected_type = unexpected<error_type>;

    // constructors
    constexpr expected() : value_{error_type()} {}
    constexpr expected(const ValueType& value) : value_{value} {}
    constexpr expected(ValueType&& value) : value_{std::move(value)} {}
    // constexpr expected(unexpected_type&& ut) : value_{std::move(ut.error())}
    // {}

    template<typename OtherError>
    constexpr expected(const unexpected<OtherError>& ut) : value_{ut.error()} {}

    template<typename OtherError>
    constexpr expected(unexpected<OtherError>&& ut) :
        value_{std::move(ut.error())} {}

    constexpr expected(const expected& other) : value_(other.value_) {}
    constexpr expected(expected&& other) : value_(std::move(other.value_)) {}

    // observers
    bool has_value() const noexcept {
        return std::holds_alternative<value_type>(value_);
    }

    bool has_error() const noexcept {
        return std::holds_alternative<error_type>(value_);
    }

    // accessors
    value_type& value() & noexcept {
        return const_cast<value_type&>(std::as_const(*this).value());
    }
    value_type&& value() && noexcept {
        return const_cast<value_type&&>(std::as_const(*this).value());
    }
    const value_type& value() const& noexcept {
        return std::get<value_type>(value_);
    }
    const value_type&& value() const&& noexcept {
        return std::move(std::get<value_type>(value_));
    }

    error_type& error() & noexcept {
        return const_cast<error_type&>(std::as_const(*this).error());
    }
    error_type&& error() && noexcept {
        return const_cast<error_type&&>(std::as_const(*this).error());
    }
    const error_type&& error() const&& noexcept {
        return std::move(std::get<error_type>(value_));
    }
    const error_type& error() const& noexcept {
        return std::get<error_type>(value_);
    }

    unexpected_type as_unexpected() const noexcept { return error(); }

    // operators
    value_type& operator*() & noexcept { return value(); }
    value_type&& operator*() && noexcept { return value(); }
    const value_type& operator*() const& noexcept { return value(); }
    const value_type&& operator*() const&& noexcept { return value(); }
    value_type& operator->() noexcept { return value(); }
    const value_type& operator->() const noexcept { return value(); }
    operator unexpected_type() const noexcept { return error(); }
    operator bool() const noexcept { return has_value(); }

    expected& operator=(const expected& other) noexcept {
        value_ = other.value_;
    }
    expected& operator=(expected&& other) noexcept {
        std::swap(value_, other.value_);
    }

    template<typename OtherError>
    expected& operator=(const unexpected<OtherError>& other) noexcept {
        value_ = other.value_;
        return *this;
    }

    template<typename OtherError>
    expected& operator=(unexpected<OtherError>&& other) noexcept {
        return (*this ? (void)*this = other : std::swap(value_, other.error())),
               *this;
    }

    template<typename OtherType, typename OtherError>
    expected& operator=(const expected<OtherType, OtherError>& other) noexcept {
        other ? value_ = other.value() : value_ = other.error();
        return *this;
    }

    template<typename OtherType>
    expected& operator=(const OtherType& other) noexcept {
        value_ = other;
        return *this;
    }

private:
    std::variant<value_type, error_type> value_;
};

} // namespace stdx
