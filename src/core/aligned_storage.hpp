#pragma once

#include <cstddef>
#include "lzdef.hpp"

namespace lzh {

template<std::size_t Len, std::size_t Align>
struct aligned_storage {
    struct type {
        alignas(Align) lzh::byte data[Len];
    };
};

template<std::size_t Len, std::size_t Align>
using aligned_storage_t = typename aligned_storage<Len, Align>::type;

} // namespace lzh
