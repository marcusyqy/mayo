#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>

#define ZOO_ASSERT_COND(condition) assert(condition)
#define ZOO_ASSERT_COND_MESSAGE(condition, message) assert((condition) && message)

#define __GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define ZOO_ASSERT(...) __GET_3RD_ARG(__VA_ARGS__, ZOO_ASSERT_COND_MESSAGE, ZOO_ASSERT_COND)(__VA_ARGS__)


// should this be in constants?
namespace zoo {

// signed numbers
using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

// unsigned numbers
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// float numbers
using f32 = float;
using f64 = double;

namespace core {

struct Version {
    s16 major;
    s16 minor;
    s16 patch;
};

namespace engine {

constexpr core::Version version = { 0, 0, 0 };
// should i remove this data?
constexpr std::string_view name = "Zoo Engine";

} // namespace engine

} // namespace core

} // namespace zoo
