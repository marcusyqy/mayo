#pragma once

#include "Input.hpp"
#include "core/Log.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo::input::glfw_layer {

KeyCode convert(input::KeyCode) noexcept {
    ZOO_LOG_ERROR("convert function for glfw key code not implemented yet!");
    return {};
}

input::KeyCode convert(KeyCode value) noexcept {

    switch (value.key_) {
    case GLFW_KEY_ESCAPE:
        if (value.action_ == GLFW_PRESS) {
            return input::KeyCode{Key::escape, Action::pressed, Mod::none};
        }
    }

    ZOO_LOG_ERROR("not implemented feature for input");
    return input::KeyCode{};
}

} // namespace zoo::input::glfw_layer
