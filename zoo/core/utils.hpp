#pragma once

#include "fwd.hpp"
#include "stdx/expected.hpp"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace zoo::core {

void check_memory() noexcept;
stdx::expected<std::string, std::runtime_error> read_file(std::string_view filename) noexcept;

} // namespace zoo::core
  //
  //
namespace zoo {
void* ptr_round_up_align(void* ptr, uintptr_t align) noexcept;
void* ptr_round_down_align(void* ptr, uintptr_t align) noexcept;
} // namespace zoo
