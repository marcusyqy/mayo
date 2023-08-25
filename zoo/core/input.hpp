#pragma once

namespace zoo {

enum class Key {
    // clang-format off
    unknown,
    n_0, n_1, n_2, n_3, n_4, n_5, n_6, n_7, n_8, n_9,
    a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
    space,
    apostrophe,
    comma,
    minus,
    period,
    slash,
    semicolon,
    equal,
    left_bracket,
    backslash,
    right_bracket,
    grave_accent,
    escape,
    enter,
    tab,
    backspace,
    insert,
    del,
    right,
    left,
    down,
    up,
    page_up,
    page_down,
    home,
    end,
    caps_lock,
    scroll_lock,
    num_lock,
    print_screen,
    pause,
    f1, f2, f3, f4,
    f5, f6, f7, f8,
    f9, f10, f11, f12,
    f13, f14, f15, f16,
    f17, f18, f19, f20,
    f21, f22, f23, f24, f25,

    kp_0, kp_1, kp_2, kp_3, kp_4, kp_5, kp_6, kp_7, kp_8, kp_9,
    kp_decimal,
    kp_divide,
    kp_multiply,
    kp_subtract,
    kp_add,
    kp_enter,
    kp_equal,

    menu,
    none,
    MAX_COUNT
    // clang-format on
};

// this should probably be a bit operation
enum class Mod { none, l_shift, l_control, l_alt, l_super, r_shift, r_control, r_alt, r_super, MAX_COUNT };

enum class Action { pressed, released, repeat, none, MAX_COUNT };

struct Key_Code {
    Key key;
    Action action;
    Mod mod = Mod::none;
};

namespace glfw_layer {

struct Glfw_Key_Code {
    int key;
    int scancode;
    int action;
    int mods;
};

Glfw_Key_Code convert(Key_Code value) noexcept;
Key_Code convert(Glfw_Key_Code value) noexcept;

} // namespace glfw_layer

} // namespace zoo
