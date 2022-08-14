#pragma once

namespace zoo {

namespace input {

    enum class Key {
        // clang-format off
        UNKNOWN,
        NR_0, NR_1, NR_2, NR_3, NR_4, NR_5, NR_6, NR_7, NR_8, NR_9,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        SPACE,

        APOSTROPHE,
        COMMA,
        MINUS,
        PERIOD,
        SLASH,
        SEMICOLON,
        EQUAL,
        LEFT_BRACKET,
        BACKSLASH,
        RIGHT_BRACKET,
        GRAVE_ACCENT,
        ESCAPE,
        ENTER,
        TAB,
        BACKSPACE,
        INSERT,
        DELETE,
        RIGHT,
        LEFT,
        DOWN,
        UP,
        PAGE_UP,
        PAGE_DOWN,
        HOME,
        END,
        CAPS_LOCK,
        SCROLL_LOCK,
        NUM_LOCK,
        PRINT_SCREEN,
        PAUSE,

        F1, F2, F3, F4,
        F5, F6, F7, F8,
        F9, F10, F11, F12,
        F13, F14, F15, F16,
        F17, F18, F19, F20,
        F21, F22, F23, F24, F25,

        KP_0, KP_1, KP_2, KP_3, KP_4, KP_5, KP_6, KP_7, KP_8, KP_9,
        KP_DECIMAL,
        KP_DIVIDE,
        KP_MULTIPLY,
        KP_SUBTRACT,
        KP_ADD,
        KP_ENTER,
        KP_EQUAL,

        MENU,
        LAST
        // clang-format on
    };

    enum class Mod {
        LEFT_SHIFT,
        LEFT_CONTROL,
        LEFT_ALT,
        LEFT_SUPER,
        RIGHT_SHIFT,
        RIGHT_CONTROL,
        RIGHT_ALT,
        RIGHT_SUPER,
        LAST
    };

    struct KeyCode {
        Mod mod_;
        Key code_;
    }

};

} // namespace zoo