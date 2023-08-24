#pragma once

#include "aligned_storage.hpp"
#include "detail/singleton.hpp"
#include <utility>

#include "core/fwd.hpp"

namespace zoo {

struct Default_Allocator {
    void* alloc(s32 size, s32 alignment) noexcept;
    void free(void* memory) noexcept;
};

template <typename Type, s32 N>
struct Bucket {
    using uninitialized_type = typed_aligned_storage_t<Type>;
    using value_type         = Type;

    uninitialized_type storage[N];
    s32 count{};

    // Helper functions.
    value_type& initialized(s32 idx) { return *reinterpret_cast<value_type*>(storage + idx); }
    const value_type& initialized(s32 idx) const { return *reinterpret_cast<const value_type*>(storage + idx); }
    uninitialized_type& uninitalized(s32 idx) { return storage[idx]; }

    template <typename... Args>
    void construct_at(s32 idx, Args&&... args) {
        new (storage + idx) Type{ std::forward<Args&&>(args)... };
    }

    template <typename... Args>
    void destroy_at(s32 idx) {
        using decayed_type = std::decay_t<value_type>;
        // call destructor
        (storage + idx)->~decayed_type();
    }
};

// This is a managed array that never resizes.
// For when we know the maximum size of the array.
template <typename Type, s32 N>
struct Array : public Bucket<Type, N> {

    using Self = Bucket<Type, N>;

    Type& operator[](s32 idx) noexcept { return const_cast<Type&>(std::as_const(*this)[idx]); }

    const Type& operator[](s32 idx) const noexcept {
        ZOO_ASSERT(idx < Self::count, "Operator[] has idx that goes above count");

        auto storage_t = reinterpret_cast<Type*>(+Self::storage);
        // TODO: check if we need to launder this.
        return std::launder(storage_t)[idx];
    }

    void push(Type&& value) noexcept { emplace(std::move(value)); }

    template <typename... Args>
    void emplace(Args&&... args) noexcept {
        ZOO_ASSERT(
            Self::count <= N,
            "Seems like the current index has gone way past "
            "the storage size? If you require some sort of "
            "dynamic array you consider Bucket_Arra");
        new (Self::storage + Self::count++) Type{ std::forward<Args&&>(args)... };
    }

    void pop() noexcept {
        ZOO_ASSERT(Self::count != 0, "Have not been pushed so cannot be pop-ed");
        Self::count--;
        std::destroy_at(Self::storage + Self::count);
    }

    void clear() noexcept {
        auto storage = std::launder(reinterpret_cast<Type*>(+Self::storage));
        for (s32 i = 0; i < Self::count; ++i) {
            std::destroy_at(storage + i);
        }
        Self::count = 0;
    }

    s32 size() const noexcept { return Self::count; }

    template <typename... TArgs>
    void resize(const s32 n, TArgs&&... args) noexcept {
        ZOO_ASSERT(n <= N, "resize target must at least be lesser or equal to size of Array");

        if (n < Self::count) {
            for (s32 i = n; i < Self::count; ++i) {
                pop();
            }
        } else if (n > Self::count) {
            for (s32 i = Self::count; i < n; ++i) {
                emplace(std::forward<TArgs&&>(args)...);
            }
        } else {
            return;
        }
        Self::count = n;
    }

public: // all the operators and constructors
    Array(const Array& other) noexcept { *this = other; }
    Array(Array&& other) noexcept { *this = std::move(other); }

    Array& operator=(const Array& other) noexcept {
        auto storage_t       = reinterpret_cast<Type*>(+Self::storage);
        auto other_storage_t = reinterpret_cast<Type*>(+other.storage);

        // copy each of the variables 1 by 1
        s32 i = 0;
        for (; i < Self::count && i < other.count; ++i) {
            storage_t[i] = other_storage_t[i];
        }

        // initialize more
        for (; i < other.count; ++i) {
            new (Self::storage + i) Type{ other_storage_t[i] };
        }

        auto laundered = std::launder(storage_t);
        for (; i < Self::count; ++i) {
            std::destroy_at(laundered + i);
        }

        Self::count = other.count;

        return *this;
    }

    Array& operator=(Array&& other) noexcept {
        auto storage_t       = reinterpret_cast<Type*>(+Self::storage);
        auto other_storage_t = reinterpret_cast<Type*>(+other.storage);

        // NOTE: this looks exactly the same as the copy assignment due to not
        // wanting any additional overhead when evaluating this kinds of
        // operator= move each of the variables 1 by 1
        s32 i = 0;
        for (; i < Self::count && i < other.count; ++i) {
            storage_t[i] = std::move(other_storage_t[i]);
        }

        // initialize more
        for (; i < other.count; ++i) {
            new (Self::storage + i) Type{ std::move(other_storage_t[i]) };
        }

        auto laundered = std::launder(storage_t);
        for (; i < Self::count; ++i) {
            std::destroy_at(laundered + i);
        }

        Self::count = other.count;

        other.clear();
        return *this;
    }

    Array() noexcept = default;
    ~Array() noexcept { clear(); }
};

template <typename T, typename Allocator = Default_Allocator>
class Allocated_Array {
public:
    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    s32 size() const noexcept { return size_; }

    template <typename... Args>
    Allocated_Array(s32 size, Allocator& allocator, Args&&... args) noexcept :
        allocator_(&allocator),
        data_(new(allocator_->allocate(sizeof(T) * size, alignof(T))) T{ std::forward<Args&&>(args)... }), size_(size) {
    }

    template <typename... Args>
    Allocated_Array(s32 size, Args&&... args) noexcept :
        Allocated_Array(size, utils::Singleton<Allocator>::get_instance(), std::forward<Args&&>(args)...) {}

    ~Allocated_Array() noexcept {
        if (data_ != nullptr && size_ != 0) allocator_->free(data_);
    }

    Allocated_Array& operator=(const Allocated_Array&) noexcept {}
    Allocated_Array(const Allocated_Array&) noexcept {}
    Allocated_Array& operator=(Allocated_Array&&) noexcept {}
    Allocated_Array(Allocated_Array&&) noexcept {}

private:
    Allocator* allocator_;

    T* data_;
    s32 size_;
};

// Not ready for use.
template <typename Type, s32 N>
class Bucket_Array {
    // use a linked list to make the bucket array
    using Bucket_Type = Bucket<Type, N>;

    struct Bucket_Node {
        Bucket_Type data;
        Bucket_Node* next;
        Bucket_Node* prev;
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

    void deinit(Bucket_Type& bucket) {
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
    Bucket_Node* head;
    s32 count;
};

} // namespace zoo
