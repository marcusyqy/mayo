#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>

namespace zoo::core {

struct Version {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
};

namespace engine {
constexpr core::Version version = {0, 0, 0};
constexpr std::string_view name = "zoo_engine";
} // namespace engine

} // namespace zoo::core

#define ZOO_ASSERT(condition, message) assert((condition) && message)
