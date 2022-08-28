#pragma once
#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace zoo {

template<typename T>
concept Referenceable = requires(T a) {
    std::is_copy_constructible_v<T>&& std::is_move_constructible_v<T>&&
        std::is_copy_assignable_v<T>&& std::is_move_assignable_v<T>;
};

template<Referenceable T>
class RefStorage {
public:
private:
    std::vector<Type> storage_;
};

template<Referenceable T>
class Ref {
public:
    using Storage = RefStorage<T>;

    Ref(std::shared_ptr<Storage> storage, std::size_t index) : storage_(std::move(storage), index_(index) {
	}

 private:
     std::shared_ptr<Storage> storage_;
     std::size_t index_;
};

template<Referenceable T>
Ref(std::shared_ptr<RefStorage<T>> type, std::size_t index) -> Ref<T>;

} // namespace zoo
