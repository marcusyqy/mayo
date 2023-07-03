#pragma once
#include <memory>
#include <variant>
#include <vector>

#include "node.hpp"

namespace refl {

namespace detail {

struct erase_info {
    const size_t alignment;
    const size_t size;
    void* (*const default_constructor)(void*);
    void* (*const copy_constructor)(void*, void*);
    void* (*const move_constructor)(void*, void*);
    void (*const destructor)(void*);
};

enum class function_type { constructor, destructor, member, op };

template<typename Type>
struct static_info {
    static constexpr size_t alignment = alignof(Type);
    static constexpr size_t size = sizeof(Type);

    static constexpr auto default_constructor = [](void* ptr) -> void* {
        return new (std::launder(reinterpret_cast<Type*>(ptr))) Type{};
    };

    static constexpr auto copy_constructor = [](void* ptr,
                                                 void* other) -> void* {
        return new (std::launder(reinterpret_cast<Type*>(ptr)))
            Type{ *(static_cast<Type*>(other)) };
    };

    static constexpr auto move_constructor = [](void* ptr,
                                                 void* other) -> void* {
        return new (std::launder(reinterpret_cast<Type*>(ptr)))
            Type{ std::move(*(static_cast<Type*>(other))) };
    };

    static constexpr auto destructor = [](void* ptr) {
        std::destroy_at(std::launder(reinterpret_cast<Type*>(ptr)));
    };

    static erase_info to_erase_info() noexcept {
        return { alignment, size, default_constructor, copy_constructor,
            move_constructor, destructor };
    }

    operator erase_info() const noexcept { return to_erase_info(); }
};

struct meta_any {};

struct any {
    erase_info info_;
};

enum class type_id {
    object,
    i8,
    i16,
    i32,
    i64,
    f32,
    f64,
    pointer,
    reference, // same as pointer but maybe we need to denote the dereference
    _stdx_enum_as_bitmask
};

} // namespace detail

struct any_ref {
private:
    detail::any ctx_;
    void* ref_;
};

struct any {
private:
    detail::any ctx_;
    void* actual_;
};

} // namespace refl
