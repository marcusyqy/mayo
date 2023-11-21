#pragma once
#include "basic.hpp"

// doesn't own the memory.
struct Linear_Allocator {
    void* memory = {};
    size_t size  = {};

    // state
    size_t curr_offset = {};
    size_t prev_offset = {};
};

void* allocate(Linear_Allocator& allocator, size_t size, size_t alignment = 2 * sizeof(void*)) noexcept {
    assert(is_power_of_two(alignment));
    uintptr_t current = (uintptr_t)allocator.memory + (uintptr_t)allocator.curr_offset;

    return nullptr;
}
