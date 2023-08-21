#pragma once

#include <type_traits>
#include <utility>
#include <cstdint>
#include "fwd.hpp"
#include  <exception>
#include "stdx/expected.hpp"
#include <stdexcept>

namespace zoo::core {

// TODO: move this to a utils :: namespace or something.
template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
T is_power_of_two(T v) noexcept {
    return (((v) != 0) && (((v) & ((v)-1)) == 0));
}

void* ptr_round_up_align(void* ptr, uintptr_t align) noexcept;
void* ptr_round_down_align(void* ptr, uintptr_t align) noexcept;

void attach_debug() noexcept;

stdx::expected<std::string, std::runtime_error> read_file(std::string_view filename) noexcept;

} // namespace zoo::core
