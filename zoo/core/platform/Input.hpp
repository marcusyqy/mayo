#pragma once

namespace zoo::input {

enum class Key {
    // clang-format off
    unknown,
    nr_0, nr_1, nr_2, nr_3, nr_4, nr_5, nr_6, nr_7, nr_8, nr_9,
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
    last
    // clang-format on
};

// this should probably be a bit operation
enum class Mod {
    none,
    left_shift,
    left_control,
    left_alt,
    left_super,
    right_shift,
    right_control,
    right_alt,
    right_super,
    last
};

enum class Action { pressed, released, repeat };

struct KeyCode {
    Key key_;
    Action action_;
    Mod mod_ = Mod::none;
};

namespace glfw_layer {

struct KeyCode {
    int key_;
    int scancode_;
    int action_;
    int mods_;
};

KeyCode convert(input::KeyCode value) noexcept;
input::KeyCode convert(KeyCode value) noexcept;

} // namespace glfw_layer

} // namespace zoo::input
