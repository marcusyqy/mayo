#pragma once

#include <exception>
#include <stdexcept>
#include <string_view>

#include <stdx/expected.hpp>

namespace zoo {

stdx::expected<std::string, std::runtime_error> read_file(std::string_view filename) noexcept;

} // namespace zoo
