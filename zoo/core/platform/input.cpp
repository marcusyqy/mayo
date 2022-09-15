#pragma once

#include "input.hpp"
#include "core/log.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo::input::glfw_layer {

auto convert(input::key_code) noexcept -> key_code {
    ZOO_LOG_ERROR("convert function for glfw key code not implemented yet!");
    return {};
}

auto convert(key_code value) noexcept -> input::key_code {

    switch (value.key_) {
    case GLFW_KEY_ESCAPE:
        if (value.action_ == GLFW_PRESS) {
            return input::key_code{key::escape, action::pressed, mod::none};
        }
    }

    ZOO_LOG_ERROR(
        "convert function for keycode not implemented feature for input");
    return input::key_code{};
}

} // namespace zoo::input::glfw_layer
