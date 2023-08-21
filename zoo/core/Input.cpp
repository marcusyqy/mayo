#include "Input.hpp"
#include "core/Log.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace zoo::input::glfw_layer {

namespace {
template <Key key>
struct zoo_to_glfw {
    static constexpr int value = GLFW_KEY_UNKNOWN;
};

template <Key key>
static constexpr auto zoo_to_glfw_v = zoo_to_glfw<key>::value;

template <int key>
struct glfw_to_zoo {
    static constexpr Key value = Key::none;
};

template <int key>
static constexpr auto glfw_to_zoo_v = glfw_to_zoo<key>::value;

#define INPUT_PAIR(key, glfw_key)                                                                                      \
    template <>                                                                                                        \
    struct zoo_to_glfw<key> {                                                                                          \
        static constexpr int value = glfw_key;                                                                         \
    };                                                                                                                 \
                                                                                                                       \
    template <>                                                                                                        \
    struct glfw_to_zoo<glfw_key> {                                                                                     \
        static constexpr Key value = key;                                                                              \
    };

INPUT_PAIR(Key::space, GLFW_KEY_SPACE)
INPUT_PAIR(Key::apostrophe, GLFW_KEY_APOSTROPHE)
INPUT_PAIR(Key::comma, GLFW_KEY_COMMA)
INPUT_PAIR(Key::minus, GLFW_KEY_MINUS)
INPUT_PAIR(Key::period, GLFW_KEY_PERIOD)
INPUT_PAIR(Key::slash, GLFW_KEY_SLASH)
INPUT_PAIR(Key::n_0, GLFW_KEY_0)
INPUT_PAIR(Key::n_1, GLFW_KEY_1)
INPUT_PAIR(Key::n_2, GLFW_KEY_2)
INPUT_PAIR(Key::n_3, GLFW_KEY_3)
INPUT_PAIR(Key::n_4, GLFW_KEY_4)
INPUT_PAIR(Key::n_5, GLFW_KEY_5)
INPUT_PAIR(Key::n_6, GLFW_KEY_6)
INPUT_PAIR(Key::n_7, GLFW_KEY_7)
INPUT_PAIR(Key::n_8, GLFW_KEY_8)
INPUT_PAIR(Key::n_9, GLFW_KEY_9)
INPUT_PAIR(Key::semicolon, GLFW_KEY_SEMICOLON)
INPUT_PAIR(Key::equal, GLFW_KEY_EQUAL)
INPUT_PAIR(Key::a, GLFW_KEY_A)
INPUT_PAIR(Key::b, GLFW_KEY_B)
INPUT_PAIR(Key::c, GLFW_KEY_C)
INPUT_PAIR(Key::d, GLFW_KEY_D)
INPUT_PAIR(Key::e, GLFW_KEY_E)
INPUT_PAIR(Key::f, GLFW_KEY_F)
INPUT_PAIR(Key::g, GLFW_KEY_G)
INPUT_PAIR(Key::h, GLFW_KEY_H)
INPUT_PAIR(Key::i, GLFW_KEY_I)
INPUT_PAIR(Key::j, GLFW_KEY_J)
INPUT_PAIR(Key::k, GLFW_KEY_K)
INPUT_PAIR(Key::l, GLFW_KEY_L)
INPUT_PAIR(Key::m, GLFW_KEY_M)
INPUT_PAIR(Key::n, GLFW_KEY_N)
INPUT_PAIR(Key::o, GLFW_KEY_O)
INPUT_PAIR(Key::p, GLFW_KEY_P)
INPUT_PAIR(Key::q, GLFW_KEY_Q)
INPUT_PAIR(Key::r, GLFW_KEY_R)
INPUT_PAIR(Key::s, GLFW_KEY_S)
INPUT_PAIR(Key::t, GLFW_KEY_T)
INPUT_PAIR(Key::u, GLFW_KEY_U)
INPUT_PAIR(Key::v, GLFW_KEY_V)
INPUT_PAIR(Key::w, GLFW_KEY_W)
INPUT_PAIR(Key::x, GLFW_KEY_X)
INPUT_PAIR(Key::y, GLFW_KEY_Y)
INPUT_PAIR(Key::z, GLFW_KEY_Z)
INPUT_PAIR(Key::left_bracket, GLFW_KEY_LEFT_BRACKET)
INPUT_PAIR(Key::backslash, GLFW_KEY_BACKSLASH)
INPUT_PAIR(Key::right_bracket, GLFW_KEY_RIGHT_BRACKET)
INPUT_PAIR(Key::grave_accent, GLFW_KEY_GRAVE_ACCENT)
INPUT_PAIR(Key::escape, GLFW_KEY_ESCAPE)
INPUT_PAIR(Key::enter, GLFW_KEY_ENTER)
INPUT_PAIR(Key::tab, GLFW_KEY_TAB)
INPUT_PAIR(Key::backspace, GLFW_KEY_BACKSPACE)
INPUT_PAIR(Key::insert, GLFW_KEY_INSERT)
INPUT_PAIR(Key::del, GLFW_KEY_DELETE)
INPUT_PAIR(Key::right, GLFW_KEY_RIGHT)
INPUT_PAIR(Key::left, GLFW_KEY_LEFT)
INPUT_PAIR(Key::down, GLFW_KEY_DOWN)
INPUT_PAIR(Key::up, GLFW_KEY_UP)
INPUT_PAIR(Key::page_up, GLFW_KEY_PAGE_UP)
INPUT_PAIR(Key::page_down, GLFW_KEY_PAGE_DOWN)
INPUT_PAIR(Key::home, GLFW_KEY_HOME)
INPUT_PAIR(Key::end, GLFW_KEY_END)
INPUT_PAIR(Key::caps_lock, GLFW_KEY_CAPS_LOCK)
INPUT_PAIR(Key::scroll_lock, GLFW_KEY_SCROLL_LOCK)
INPUT_PAIR(Key::num_lock, GLFW_KEY_NUM_LOCK)
INPUT_PAIR(Key::print_screen, GLFW_KEY_PRINT_SCREEN)
INPUT_PAIR(Key::pause, GLFW_KEY_PAUSE)
INPUT_PAIR(Key::f1, GLFW_KEY_F1)
INPUT_PAIR(Key::f2, GLFW_KEY_F2)
INPUT_PAIR(Key::f3, GLFW_KEY_F3)
INPUT_PAIR(Key::f4, GLFW_KEY_F4)
INPUT_PAIR(Key::f5, GLFW_KEY_F5)
INPUT_PAIR(Key::f6, GLFW_KEY_F6)
INPUT_PAIR(Key::f7, GLFW_KEY_F7)
INPUT_PAIR(Key::f8, GLFW_KEY_F8)
INPUT_PAIR(Key::f9, GLFW_KEY_F9)
INPUT_PAIR(Key::f10, GLFW_KEY_F10)
INPUT_PAIR(Key::f11, GLFW_KEY_F11)
INPUT_PAIR(Key::f12, GLFW_KEY_F12)
INPUT_PAIR(Key::f13, GLFW_KEY_F13)
INPUT_PAIR(Key::f14, GLFW_KEY_F14)
INPUT_PAIR(Key::f15, GLFW_KEY_F15)
INPUT_PAIR(Key::f16, GLFW_KEY_F16)
INPUT_PAIR(Key::f17, GLFW_KEY_F17)
INPUT_PAIR(Key::f18, GLFW_KEY_F18)
INPUT_PAIR(Key::f19, GLFW_KEY_F19)
INPUT_PAIR(Key::f20, GLFW_KEY_F20)
INPUT_PAIR(Key::f21, GLFW_KEY_F21)
INPUT_PAIR(Key::f22, GLFW_KEY_F22)
INPUT_PAIR(Key::f23, GLFW_KEY_F23)
INPUT_PAIR(Key::f24, GLFW_KEY_F24)
INPUT_PAIR(Key::f25, GLFW_KEY_F25)
INPUT_PAIR(Key::kp_0, GLFW_KEY_KP_0)
INPUT_PAIR(Key::kp_1, GLFW_KEY_KP_1)
INPUT_PAIR(Key::kp_2, GLFW_KEY_KP_2)
INPUT_PAIR(Key::kp_3, GLFW_KEY_KP_3)
INPUT_PAIR(Key::kp_4, GLFW_KEY_KP_4)
INPUT_PAIR(Key::kp_5, GLFW_KEY_KP_5)
INPUT_PAIR(Key::kp_6, GLFW_KEY_KP_6)
INPUT_PAIR(Key::kp_7, GLFW_KEY_KP_7)
INPUT_PAIR(Key::kp_8, GLFW_KEY_KP_8)
INPUT_PAIR(Key::kp_9, GLFW_KEY_KP_9)
INPUT_PAIR(Key::kp_decimal, GLFW_KEY_KP_DECIMAL)
INPUT_PAIR(Key::kp_divide, GLFW_KEY_KP_DIVIDE)
INPUT_PAIR(Key::kp_multiply, GLFW_KEY_KP_MULTIPLY)
INPUT_PAIR(Key::kp_subtract, GLFW_KEY_KP_SUBTRACT)
INPUT_PAIR(Key::kp_add, GLFW_KEY_KP_ADD)
INPUT_PAIR(Key::kp_enter, GLFW_KEY_KP_ENTER)
INPUT_PAIR(Key::kp_equal, GLFW_KEY_KP_EQUAL)

// TODO: find out how to include these
// INPUT_PAIR(Key::world, GLFW_KEY_WORLD_1)
// INPUT_PAIR(Key::world, GLFW_KEY_WORLD_2)
// INPUT_PAIR(Key::enter, GLFW_KEY_LEFT_SHIFT)
// INPUT_PAIR(Key::enter, GLFW_KEY_LEFT_CONTROL)
// INPUT_PAIR(Key::enter, GLFW_KEY_LEFT_ALT)
// INPUT_PAIR(Key::enter, GLFW_KEY_LEFT_SUPER)
// INPUT_PAIR(Key::enter, GLFW_KEY_RIGHT_SHIFT)
// INPUT_PAIR(Key::enter, GLFW_KEY_RIGHT_CONTROL)
// INPUT_PAIR(Key::enter, GLFW_KEY_RIGHT_ALT)
// INPUT_PAIR(Key::enter, GLFW_KEY_RIGHT_SUPER)
// INPUT_PAIR(Key::enter, GLFW_KEY_MENU)

#undef INPUT_PAIR

} // namespace

KeyCode convert(input::KeyCode) noexcept {
    ZOO_LOG_ERROR("convert function for glfw key code not implemented yet!");
    return {};
}

input::KeyCode convert(KeyCode value) noexcept {
    Key key       = Key::none;
    Action action = Action::none;
    Mod mod       = Mod::none;

    switch (value.key_) {
#define SWITCH_KEY(xkey)                                                                                               \
    case xkey: key = glfw_to_zoo_v<xkey>; break;
        SWITCH_KEY(GLFW_KEY_UNKNOWN)
        SWITCH_KEY(GLFW_KEY_SPACE)
        SWITCH_KEY(GLFW_KEY_APOSTROPHE)
        SWITCH_KEY(GLFW_KEY_COMMA)
        SWITCH_KEY(GLFW_KEY_MINUS)
        SWITCH_KEY(GLFW_KEY_PERIOD)
        SWITCH_KEY(GLFW_KEY_SLASH)
        SWITCH_KEY(GLFW_KEY_0)
        SWITCH_KEY(GLFW_KEY_1)
        SWITCH_KEY(GLFW_KEY_2)
        SWITCH_KEY(GLFW_KEY_3)
        SWITCH_KEY(GLFW_KEY_4)
        SWITCH_KEY(GLFW_KEY_5)
        SWITCH_KEY(GLFW_KEY_6)
        SWITCH_KEY(GLFW_KEY_7)
        SWITCH_KEY(GLFW_KEY_8)
        SWITCH_KEY(GLFW_KEY_9)
        SWITCH_KEY(GLFW_KEY_SEMICOLON)
        SWITCH_KEY(GLFW_KEY_EQUAL)
        SWITCH_KEY(GLFW_KEY_A)
        SWITCH_KEY(GLFW_KEY_B)
        SWITCH_KEY(GLFW_KEY_C)
        SWITCH_KEY(GLFW_KEY_D)
        SWITCH_KEY(GLFW_KEY_E)
        SWITCH_KEY(GLFW_KEY_F)
        SWITCH_KEY(GLFW_KEY_G)
        SWITCH_KEY(GLFW_KEY_H)
        SWITCH_KEY(GLFW_KEY_I)
        SWITCH_KEY(GLFW_KEY_J)
        SWITCH_KEY(GLFW_KEY_K)
        SWITCH_KEY(GLFW_KEY_L)
        SWITCH_KEY(GLFW_KEY_M)
        SWITCH_KEY(GLFW_KEY_N)
        SWITCH_KEY(GLFW_KEY_O)
        SWITCH_KEY(GLFW_KEY_P)
        SWITCH_KEY(GLFW_KEY_Q)
        SWITCH_KEY(GLFW_KEY_R)
        SWITCH_KEY(GLFW_KEY_S)
        SWITCH_KEY(GLFW_KEY_T)
        SWITCH_KEY(GLFW_KEY_U)
        SWITCH_KEY(GLFW_KEY_V)
        SWITCH_KEY(GLFW_KEY_W)
        SWITCH_KEY(GLFW_KEY_X)
        SWITCH_KEY(GLFW_KEY_Y)
        SWITCH_KEY(GLFW_KEY_Z)
        SWITCH_KEY(GLFW_KEY_LEFT_BRACKET)
        SWITCH_KEY(GLFW_KEY_BACKSLASH)
        SWITCH_KEY(GLFW_KEY_RIGHT_BRACKET)
        SWITCH_KEY(GLFW_KEY_GRAVE_ACCENT)
        SWITCH_KEY(GLFW_KEY_WORLD_1)
        SWITCH_KEY(GLFW_KEY_WORLD_2)
        SWITCH_KEY(GLFW_KEY_ESCAPE)
        SWITCH_KEY(GLFW_KEY_ENTER)
        SWITCH_KEY(GLFW_KEY_TAB)
        SWITCH_KEY(GLFW_KEY_BACKSPACE)
        SWITCH_KEY(GLFW_KEY_INSERT)
        SWITCH_KEY(GLFW_KEY_DELETE)
        SWITCH_KEY(GLFW_KEY_RIGHT)
        SWITCH_KEY(GLFW_KEY_LEFT)
        SWITCH_KEY(GLFW_KEY_DOWN)
        SWITCH_KEY(GLFW_KEY_UP)
        SWITCH_KEY(GLFW_KEY_PAGE_UP)
        SWITCH_KEY(GLFW_KEY_PAGE_DOWN)
        SWITCH_KEY(GLFW_KEY_HOME)
        SWITCH_KEY(GLFW_KEY_END)
        SWITCH_KEY(GLFW_KEY_CAPS_LOCK)
        SWITCH_KEY(GLFW_KEY_SCROLL_LOCK)
        SWITCH_KEY(GLFW_KEY_NUM_LOCK)
        SWITCH_KEY(GLFW_KEY_PRINT_SCREEN)
        SWITCH_KEY(GLFW_KEY_PAUSE)
        SWITCH_KEY(GLFW_KEY_F1)
        SWITCH_KEY(GLFW_KEY_F2)
        SWITCH_KEY(GLFW_KEY_F3)
        SWITCH_KEY(GLFW_KEY_F4)
        SWITCH_KEY(GLFW_KEY_F5)
        SWITCH_KEY(GLFW_KEY_F6)
        SWITCH_KEY(GLFW_KEY_F7)
        SWITCH_KEY(GLFW_KEY_F8)
        SWITCH_KEY(GLFW_KEY_F9)
        SWITCH_KEY(GLFW_KEY_F10)
        SWITCH_KEY(GLFW_KEY_F11)
        SWITCH_KEY(GLFW_KEY_F12)
        SWITCH_KEY(GLFW_KEY_F13)
        SWITCH_KEY(GLFW_KEY_F14)
        SWITCH_KEY(GLFW_KEY_F15)
        SWITCH_KEY(GLFW_KEY_F16)
        SWITCH_KEY(GLFW_KEY_F17)
        SWITCH_KEY(GLFW_KEY_F18)
        SWITCH_KEY(GLFW_KEY_F19)
        SWITCH_KEY(GLFW_KEY_F20)
        SWITCH_KEY(GLFW_KEY_F21)
        SWITCH_KEY(GLFW_KEY_F22)
        SWITCH_KEY(GLFW_KEY_F23)
        SWITCH_KEY(GLFW_KEY_F24)
        SWITCH_KEY(GLFW_KEY_F25)
        SWITCH_KEY(GLFW_KEY_KP_0)
        SWITCH_KEY(GLFW_KEY_KP_1)
        SWITCH_KEY(GLFW_KEY_KP_2)
        SWITCH_KEY(GLFW_KEY_KP_3)
        SWITCH_KEY(GLFW_KEY_KP_4)
        SWITCH_KEY(GLFW_KEY_KP_5)
        SWITCH_KEY(GLFW_KEY_KP_6)
        SWITCH_KEY(GLFW_KEY_KP_7)
        SWITCH_KEY(GLFW_KEY_KP_8)
        SWITCH_KEY(GLFW_KEY_KP_9)
        SWITCH_KEY(GLFW_KEY_KP_DECIMAL)
        SWITCH_KEY(GLFW_KEY_KP_DIVIDE)
        SWITCH_KEY(GLFW_KEY_KP_MULTIPLY)
        SWITCH_KEY(GLFW_KEY_KP_SUBTRACT)
        SWITCH_KEY(GLFW_KEY_KP_ADD)
        SWITCH_KEY(GLFW_KEY_KP_ENTER)
        SWITCH_KEY(GLFW_KEY_KP_EQUAL)
        SWITCH_KEY(GLFW_KEY_LEFT_SHIFT)
        SWITCH_KEY(GLFW_KEY_LEFT_CONTROL)
        SWITCH_KEY(GLFW_KEY_LEFT_ALT)
        SWITCH_KEY(GLFW_KEY_LEFT_SUPER)
        SWITCH_KEY(GLFW_KEY_RIGHT_SHIFT)
        SWITCH_KEY(GLFW_KEY_RIGHT_CONTROL)
        SWITCH_KEY(GLFW_KEY_RIGHT_ALT)
        SWITCH_KEY(GLFW_KEY_RIGHT_SUPER)
        SWITCH_KEY(GLFW_KEY_MENU)
#undef SWITCH_KEY
    }

    switch (value.action_) {
        case GLFW_PRESS: action = Action::pressed; break;
        case GLFW_RELEASE: action = Action::released; break;
        case GLFW_REPEAT: action = Action::repeat; break;
        default: action = Action::none; break;
    }

    return input::KeyCode{ key, action, mod };
}

} // namespace zoo::input::glfw_layer
