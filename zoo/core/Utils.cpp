#include "Utils.hpp"

namespace zoo::core {

void* ptr_round_up_align(void* ptr, uintptr_t align) noexcept {
    ZOO_ASSERT(is_power_of_two(align), "align must be a power of two!");
    return (void*)(((uintptr_t)ptr + (align - 1)) & ~(align - 1));
}

void* ptr_round_down_align(void* ptr, uintptr_t align) noexcept {
    ZOO_ASSERT(is_power_of_two(align), "align must be a power of two!");
    return (void*)((uintptr_t)ptr & ~(align - 1));
}

} // namespace zoo::core
