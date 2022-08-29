#pragma once

#include "main/Info.hpp"

namespace zoo::core {

struct Version {
    uint16_t major_;
    uint16_t minor_;
    uint16_t patch_;
};

namespace engine {
constexpr Version version = {0, 0, 0};
constexpr std::string_view name = "Zoo Engine";
} // namespace engine

} // namespace zoo::core