#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>

#define ZOO_ASSERT(condition, message) assert((condition) && message)

// should this be in constants?
namespace zoo {

// signed numbers
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

// unsigned numbers
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// float numbers
using f32 = float;
using f64 = double;

namespace constants {
// defines
using size_type = s32;
using index_type = size_type;
} // namespace constants

namespace core {

struct Version {
    s16 major;
    s16 minor;
    s16 patch;
};

namespace engine {

constexpr core::Version version = {0, 0, 0};
// should i remove this data?
constexpr std::string_view name = "zoo_engine";

} // namespace engine

} // namespace core

} // namespace zoo
