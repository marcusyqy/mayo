#pragma once
#include "../types.hpp"
#include "basic.hpp"
#include <cstdint>

// Arena strategy?
struct Linear_Allocator {
    Linear_Allocator(void* buffer, size_t buffer_length) noexcept;
    void* allocate(size_t size, size_t alignment) noexcept;
    void clear() noexcept;

private:
    u8* ptr;
    size_t length;
    size_t offset;
};

struct Arena {
    void* allocate(size_t size, size_t alignment) noexcept;
    void clear() noexcept;

    ~Arena() noexcept;
    Arena(const Arena& o) noexcept            = delete;
    Arena& operator=(const Arena& o) noexcept = delete;
    Arena(Arena&& o) noexcept                 = delete;
    Arena& operator=(Arena&& o) noexcept      = delete;

private:
    Linear_Allocator strategy;
    struct Memory_Bucket {
        // can change and tweak.
        static constexpr size_t length = convert_to::kilo_bytes(10);
        u8 data[length];
        Memory_Bucket* next = nullptr;
    };

    Memory_Bucket first    = {};
    Memory_Bucket* current = nullptr;
};
