#pragma once
#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace zoo {

template<typename T>
concept referenceable = requires(T a) {
    std::is_copy_constructible_v<T>&& std::is_move_constructible_v<T>&&
        std::is_copy_assignable_v<T>&& std::is_move_assignable_v<T>;
};

template<referenceable T>
class ref_storage {
public:
private:
    std::vector<Type> Storage_;
};

template<referenceable T>
class ref {
public:
    using storage_type = ref_storage<T>;

    ref(std::shared_ptr<storage_type> Storage, std::size_t Index) : Storage_(std::move(Storage), Index_(Index) {}

 private:
     std::shared_ptr<storage_type> Storage_;
     std::size_t Index_;
};

template<referenceable T>
ref(std::shared_ptr<ref_storage<T>> Type, std::size_t Index) -> ref<T>;

} // namespace zoo
