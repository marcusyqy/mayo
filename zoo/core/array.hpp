#pragma once

// TODO: add this to some utility library native to `zoo`
#include <stdx/aligned_storage.hpp>
#include <utility>

#include "fwd.hpp"
#include "detail/Singleton.hpp"

namespace zoo {

struct DefaultAllocator {
    void* alloc(s32 size, s32 alignment) noexcept;
    void free(void* memory) noexcept;
};

template <typename Type, s32 N>
struct Bucket {
    using uninitialized_type = stdx::typed_aligned_storage_t<Type>;
    using value_type         = Type;

    uninitialized_type storage[N];
    s32 count{};
};

// This is a managed array that never resizes.
// For when we know the maximum size of the array.
template <typename Type, s32 N>
struct Array : public Bucket<Type, N> {

    using self            = Bucket<Type, N>;
    using reference       = Type&;
    using const_reference = const Type&;
    using pointer         = Type*;
    using const_pointer   = const Type*;
    using size_type       = s32;
    using index_type      = s32;

    reference operator[](index_type idx) noexcept { return const_cast<reference>(std::as_const(*this)[idx]); }

    const_reference operator[](index_type idx) const noexcept {
        ZOO_ASSERT(idx < self::count, "Operator[] has idx that goes above count");

        auto storage_t = reinterpret_cast<const_pointer>(+self::storage);
        // TODO: check if we need to launder this.
        return std::launder(storage_t)[idx];
    }

    void push(Type&& value) noexcept { emplace(std::move(value)); }

    template <typename... Args>
    void emplace(Args&&... args) noexcept {
        ZOO_ASSERT(
            self::count <= N,
            "Seems like the current index has gone way past "
            "the storage size? If you require some sort of "
            "dynamic array you consider Bucket_Arra");
        new (self::storage + self::count++) Type{ std::forward<Args&&>(args)... };
    }

    void pop() noexcept {
        ZOO_ASSERT(self::count != 0, "Have not been pushed so cannot be pop-ed");
        self::count--;
        std::destroy_at(self::storage + self::count);
    }

    void clear() noexcept {
        auto storage = std::launder(reinterpret_cast<pointer>(+self::storage));
        for (s32 i = 0; i < self::count; ++i) {
            std::destroy_at(storage + i);
        }
        self::count = 0;
    }

    size_type size() const noexcept { return self::count; }

    template <typename... TArgs>
    void resize(const s32 n, TArgs&&... args) noexcept {
        ZOO_ASSERT(n <= N, "resize target must at least be lesser or equal to size of Array");

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

public: // all the operators and constructors
    Array(const Array& other) noexcept { *this = other; }
    Array(Array&& other) noexcept { *this = std::move(other); }

    Array& operator=(const Array& other) noexcept {
        auto storage_t       = reinterpret_cast<pointer>(+self::storage);
        auto other_storage_t = reinterpret_cast<pointer>(+other.storage);

        // copy each of the variables 1 by 1
        s32 i = 0;
        for (; i < self::count && i < other.count; ++i) {
            storage_t[i] = other_storage_t[i];
        }

        // initialize more
        for (; i < other.count; ++i) {
            new (self::storage + i) Type{ other_storage_t[i] };
        }

        auto laundered = std::launder(storage_t);
        for (; i < self::count; ++i) {
            std::destroy_at(laundered + i);
        }

        self::count = other.count;

        return *this;
    }

    Array& operator=(Array&& other) noexcept {
        auto storage_t       = reinterpret_cast<pointer>(+self::storage);
        auto other_storage_t = reinterpret_cast<pointer>(+other.storage);

        // NOTE: this looks exactly the same as the copy assignment due to not
        // wanting any additional overhead when evaluating this kinds of
        // operator= move each of the variables 1 by 1
        s32 i = 0;
        for (; i < self::count && i < other.count; ++i) {
            storage_t[i] = std::move(other_storage_t[i]);
        }

        // initialize more
        for (; i < other.count; ++i) {
            new (self::storage + i) Type{ std::move(other_storage_t[i]) };
        }

        auto laundered = std::launder(storage_t);
        for (; i < self::count; ++i) {
            std::destroy_at(laundered + i);
        }

        self::count = other.count;

        other.clear();
        return *this;
    }

    Array() noexcept = default;
    ~Array() noexcept { clear(); }
};

template <typename T, typename Allocator = DefaultAllocator>
class AllocatedArray {
public:
    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    s32 size() const noexcept { return size_; }

    template <typename... Args>
    AllocatedArray(s32 size, Allocator& allocator, Args&&... args) noexcept :
        allocator_(&allocator),
        data_(new(allocator_->allocate(sizeof(T) * size, alignof(T))) T{ std::forward<Args&&>(args)... }), size_(size) {
    }

    // this should not be called and compiled if not intended.
    template <typename... Args>
    AllocatedArray(s32 size, Args&&... args) noexcept :
        AllocatedArray(
            // TODO: check if this is created when compiled.
            size,
            Singleton<Allocator>::get_instance(),
            std::forward<Args&&>(args)...) {}

    ~AllocatedArray() noexcept {
        if (data_ != nullptr && size_ != 0) allocator_->free(data_);
    }

    AllocatedArray& operator=(const AllocatedArray& other) noexcept {}
    AllocatedArray(const AllocatedArray& other) noexcept {}
    AllocatedArray& operator=(AllocatedArray&& other) noexcept {}
    AllocatedArray(AllocatedArray&& other) noexcept {}

private:
    Allocator* allocator_;

    T* data_;
    s32 size_;
};

// Not ready for use.
template <typename Type, s32 N>
class Bucket_Array {
    // use a linked list to make the bucket array
    using BucketT = Bucket<Type, N>;
    struct BucketNode {
        BucketT data;
        BucketNode* next;
    };

public:
    void resize(s32 size) noexcept {
        auto bucket_size                     = size / N;
        [[maybe_unused]] auto size_in_bucket = size % N;

        auto* bucket = head;
        for (s32 i = 0; i < bucket_size; ++i) {
            if (bucket->next == nullptr) {
                // allocate.
            }
            bucket = bucket->next;
        }
    }

    void deinit(BucketT& bucket) {
        // TODO: fix this!!
        (void)bucket;
    }

    ~Bucket_Array() noexcept {
        auto bucket = head;
        while (bucket) {
            auto b_temp = bucket;
            bucket      = bucket->next;
            deinit(b_temp->data);
            delete b_temp;
        }
    }

private:
    BucketNode* head;
    s32 count;
};

} // namespace zoo
