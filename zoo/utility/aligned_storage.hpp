#pragma once

#include "core/fwd.hpp"
#include <cstddef>

namespace zoo {

using byte = u8;

template <std::size_t Len, std::size_t Align>
struct aligned_storage {
    struct type {
        alignas(Align) byte data[Len];
    };
};

template <std::size_t Len, std::size_t Align>
using aligned_storage_t = typename aligned_storage<Len, Align>::type;

template <typename Type>
using typed_aligned_storage_t = typename aligned_storage<sizeof(Type), alignof(Type)>::type;

} // namespace zoo
