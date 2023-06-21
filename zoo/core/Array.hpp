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
    using pointer = Type*;
    using const_pointer = const Type*;
    using size_type = constants::size_type;
    using index_type = constants::index_type;

    reference operator[](index_type idx) noexcept {
        return const_cast<reference>(std::as_const(*this)[idx]);
    }

    const_reference operator[](index_type idx) const noexcept {
        ZOO_ASSERT(
            idx < self::count, "Operator[] has idx that goes above count");

        auto storage_t = reinterpret_cast<const_pointer>(+self::storage);
        // TODO: check if we need to launder this.
        return std::launder(storage_t)[idx];
    }

    void push(Type&& value) noexcept { emplace(std::move(value)); }

    template<typename... Args>
    void emplace(Args&&... args) noexcept {
        ZOO_ASSERT(self::count <= N,
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

    Array(const Array& other) noexcept { *this = other; }

    Array& operator=(const Array& other) noexcept {
        auto storage_t = reinterpret_cast<pointer>(+self::storage);
        auto other_storage_t = reinterpret_cast<pointer>(+other.storage);

        // copy each of the variables 1 by 1
        s32 i = 0;
        for (; i < self::count && i < other.count; ++i) {
            storage_t[i] = other_storage_t[i];
        }

        // initialize more
        for (; i < other.count; ++i) {
            new (self::storage + i) Type{other_storage_t[i]};
        }

        auto laundered = std::launder(storage_t);
        for (; i < self::count; ++i) {
            std::destroy_at(laundered + i);
        }

        self::count = other.count;

        return *this;
    }

    size_type size() const noexcept { return self::count; }

    template<typename... TArgs>
    void resize(const s32 n, TArgs&&... args) noexcept {
        ZOO_ASSERT(n <= N,
            "resize target must at least be lesser or equal to size of Array");

        if (n < self::count) {
            for (s32 i = n; i < self::count; ++i) {
                pop();
            }
        } else if (n > self::count) {
            for (s32 i = self::count; i < n; ++i) {
                emplace(std::forward<TArgs&&>(args)...);
            }
        } else {
            return;
        }
        self::count = n;
    }

    // Assumption: For move, the reason why we're memcpy-ing is basically that
    // we assume the user(me) will not do something stupid so `moving` is
    // essentially just overwriting the data and clearing the other side's data.
    // We actually don't have to do any new changes. HOWEVER. If the user(me)
    // decides to put a pointer to myself and require to update the pointer when
    // moving. This will create a crash.
    Array(Array&& other) noexcept {
        // should maybe move this?
        std::memcpy(other.storage, other.storage + other.count, self::storage);
        self::count = other.count;
        other.count = 0;
    }

    Array& operator=(Array&& other) noexcept {
        clear();
        // should maybe move this?
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
