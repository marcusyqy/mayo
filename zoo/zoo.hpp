#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace zoo {

using string      = std::string;
using string_view = std::string_view;

template <typename T>
using dyn_array = std::vector<T>;

template <typename K, typename V>
using hash_map = std::unordered_map<K, V>;

} // namespace zoo
