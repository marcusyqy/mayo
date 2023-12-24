#include "arena.hpp"
#include <new>

Linear_Allocator::Linear_Allocator(void* buffer, size_t buffer_length) noexcept :
    ptr((u8*)buffer), length(buffer_length), offset(0) {}

void Linear_Allocator::clear() noexcept { offset = 0; }

void* Linear_Allocator::allocate(size_t size, size_t alignment) noexcept {
    uintptr_t curr_ptr    = (uintptr_t)ptr + (uintptr_t)offset;
    uintptr_t candidate   = align_forward(curr_ptr, alignment);
    uintptr_t curr_offset = candidate - (uintptr_t)ptr;

    if (curr_offset + size > length) {
        return nullptr;
    }
    offset = curr_offset + size;
    // memset?
    // weird casts...
    // @TODO(marc) check if we need launder?
    return (void*)std::launder((u8*)candidate);
}

void Arena::clear() noexcept {
    if (first != nullptr) {
        strategy = Linear_Allocator(first->data, first->length);
    }
    current = first;
}

void* Arena::allocate(size_t size, size_t alignment) noexcept {
    void* ptr = strategy.allocate(size, alignment);
    if (ptr == nullptr) {
        // push_front
        if (current->next == nullptr) {
            current->next = new Memory_Bucket();
            current       = current->next;
            strategy      = Linear_Allocator(first->data, first->length);
        }
    }

    return ptr;
}

Arena::~Arena() noexcept {
    while (first != nullptr) {
        Memory_Bucket* bucket = first;
        first                 = first->next;
        delete bucket;
    }
}
