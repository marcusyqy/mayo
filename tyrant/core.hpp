#pragma once
#include <chrono>

struct Clock {
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point latest;
    double dt = 0.0;

    void tick();
    Clock();
};

enum Key : int {
    // Keyboard
    key_none = 0,
    key_tab,
    key_leftarrow,
    key_rightarrow,
    key_uparrow,
    key_downarrow,
    key_pageup,
    key_pagedown,
    key_home,
    key_end,
    key_insert,
    key_delete,
    key_backspace,
    key_space,
    key_enter,
    key_escape,
    key_leftctrl,
    key_leftshift,
    key_leftalt,
    key_leftsuper,
    key_rightctrl,
    key_rightshift,
    key_rightalt,
    key_rightsuper,
    key_menu,
    key_0,
    key_1,
    key_2,
    key_3,
    key_4,
    key_5,
    key_6,
    key_7,
    key_8,
    key_9,
    key_a,
    key_b,
    key_c,
    key_d,
    key_e,
    key_f,
    key_g,
    key_h,
    key_i,
    key_j,
    key_k,
    key_l,
    key_m,
    key_n,
    key_o,
    key_p,
    key_q,
    key_r,
    key_s,
    key_t,
    key_u,
    key_v,
    key_w,
    key_x,
    key_y,
    key_z,
    key_f1,
    key_f2,
    key_f3,
    key_f4,
    key_f5,
    key_f6,
    key_f7,
    key_f8,
    key_f9,
    key_f10,
    key_f11,
    key_f12,
    key_apostrophe,   // '
    key_comma,        // ,
    key_minus,        // -
    key_period,       // .
    key_slash,        // /
    key_semicolon,    // ;
    key_equal,        // =
    key_leftbracket,  // [
    key_rightbracket, // ]
    key_backslash,    // \ (for escape backslash)
    key_graveaccent,  // `
    key_capslock,
    key_scrolllock,
    key_numlock,
    key_printscreen,
    key_pause,
    key_keypad0,
    key_keypad1,
    key_keypad2,
    key_keypad3,
    key_keypad4,
    key_keypad5,
    key_keypad6,
    key_keypad7,
    key_keypad8,
    key_keypad9,
    key_keypaddecimal,
    key_keypaddivide,
    key_keypadmultiply,
    key_keypadsubtract,
    key_keypadadd,
    key_keypadenter,
    key_keypadequal,
    Key_MAX,
};

struct Key_State {
    bool key_down;
    double down_duration;
    double previous_down_duration;
};

struct IO {
    Key_State key_states[Key_MAX];
    void update(double dt);
    void set_key_state(Key key, bool key_down);

    IO();
};

struct Core {
    static Clock clock;
    static IO io;

    static void update();
};
