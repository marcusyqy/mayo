
#include "input.hpp"
#include "core/log.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo::input::glfw_layer {

key_code convert(input::key_code) noexcept {
    ZOO_LOG_ERROR("convert function for glfw key code not implemented yet!");
    return {};
}

input::key_code convert(key_code value) noexcept {
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
