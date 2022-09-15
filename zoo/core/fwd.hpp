#pragma once
#include <cstdint>
#include <string_view>

#include "main/info.hpp"

namespace zoo::core {

struct version {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
};

namespace engine {
constexpr core::version version = {0, 0, 0};
constexpr std::string_view name = "zoo_engine";
} // namespace engine

} // namespace zoo::core