#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace zoo {

using string = std::string;
using string_view = std::string_view;

template<typename T>
using dyn_array = std::vector<T>;

template<typename K, typename V>
using hash_map = std::unordered_map<K, V>;

template<typename T>
using ref = std::shared_ptr<T>;

template<typename T>
struct enable_ref_from_this : private std::enable_shared_from_this<T> {
    ref<T> ref_from_this() noexcept { return shared_from_this(); }
};

template<typename T, typename... Args>
ref<T> make_ref(Args&&... args) noexcept {
    return std::make_shared(std::forward<Args>(args)...);
}

} // namespace zoo
