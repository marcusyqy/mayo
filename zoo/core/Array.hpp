#pragma once

// TODO: add this to some utility library native to `zoo`
#include <stdx/aligned_storage.hpp>
#include <utility>

#include "fwd.hpp"

namespace zoo::core {

template<typename Type, s32 N>
struct Bucket {
    using uninitialized_type = stdx::typed_aligned_storage_t<Type>;
    using value_type = Type;

    uninitialized_type storage[N];
    s32 count{};
};

template<typename Type, s32 N>
struct Array : private Bucket<Type, N> {
    using self = Bucket<Type, N>;
    using reference = Type&;
    using const_reference = const Type&;
    using size_type = zoo::size_type;
    using index_type = zoo::index_type;

    reference operator[](index_type idx) noexcept {
        return const_cast<reference>(std::as_const((*this)[idx]));
    }

    const_reference operator[](index_type idx) const noexcept {
        ZOO_ASSERT(
            idx < self::count, "Operator[] has idx that goes above count");
        return self::storage[idx];
    }

    void push(Type&& value) noexcept { emplace(std::move(value)); }

    template<typename... Args>
    void emplace(Args&&... args) noexcept {
        ZOO_ASSERT(self::count >= N,
            "Seems like the current index has gone way past "
            "the storage size? If you require some sort of "
            "dynamic array you consider BucketArray.");
        new (self::storage + self::count++) Type{std::forward<Args&&>(args)...};
    }

    void pop() noexcept {
        ZOO_ASSERT(
            self::count != 0, "Have not been pushed so cannot be pop-ed");
        self::count--;
        std::destroy_at(self::storage + self::count);
    }

    void clear() noexcept {
        for (s32 i = 0; i < self::count; ++i) {
            std::destroy_at(self::storage + i);
        }
        self::count = 0;
    }

    Array() noexcept = default;

    Array(const Array& other) noexcept {
        auto storage_t = static_cast<Type*>(self::storage);
        auto other_storage_t = static_cast<Type*>(other.storage);

        // copy each of the variables 1 by 1
        for (s32 i = 0; i < self::count; ++i) {
        }
    }

    Array& operator=(const Array& other) noexcept { return *this; }

    Array(Array&& other) noexcept {
        std::memcpy(other.storage, other.storage + other.count, self::storage);
        self::count = other.count;
        other.count = 0;
    }

    Array& operator=(Array&& other) noexcept {
        clear();
        std::memcpy(other.storage, other.storage + other.count, self::storage);
        self::count = other.count;
        other.count = 0;
        return *this;
    }

    ~Array() noexcept { clear(); }
};

template<typename Type, s32 N>
struct BucketArray {

    // use a linked list to make the bucket array
    Bucket<Type, N>* buckets;
    s32 count;
};

} // namespace zoo::core
