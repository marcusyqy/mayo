#ifdef _WIN32

#define NO_ENTRY_POINT

#include "windows.hpp"
#include "core.hpp"
#include "render.hpp"

#include "basic.hpp"
#include "logger.hpp"
#include <algorithm>

#define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

namespace {

struct Window_Param {
    DWORD dwExStyle;
    LPCWSTR lpClassName;
    LPCWSTR lpWindowName;
    DWORD dwStyle;
    int x;
    int y;
    int nWidth;
    int nHeight;
    HWND hWndParent;
    HMENU hMenu;
    HINSTANCE hInstance;
    LPVOID lpParam;
};

struct Window_Messages {
    enum : UINT {
        create_window  = WM_USER + 0x1337,
        destroy_window = WM_USER + 0x1338,
    };
};

struct Service {
    static HWND hwnd;
    static void initialize() { static auto handle = CreateThread(0, 0, worker, 0, 0, &thread_id); }

private:
    static LRESULT CALLBACK wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
        LRESULT result = 0;

        switch (message) {
            case Window_Messages::create_window: {
                auto* p      = (Window_Param*)wparam;
                DWORD thread = (DWORD)lparam;
                auto new_window = CreateWindowExW(
                    p->dwExStyle,
                    p->lpClassName,
                    p->lpWindowName,
                    p->dwStyle,
                    p->x,
                    p->y,
                    p->nWidth,
                    p->nHeight,
                    p->hWndParent,
                    p->hMenu,
                    p->hInstance,
                    p->lpParam);
                assert(hwnd);

                SetWindowLongPtrW(new_window, GWLP_USERDATA, thread);
                result = (LRESULT)new_window;
                break;
            }
            case Window_Messages::destroy_window: {
                DestroyWindow((HWND)wparam);
                break;
            }
            default: {
                result = DefWindowProcW(window, message, wparam, lparam);
                break;
            }
        }

        return result;
    }

    static DWORD WINAPI worker(LPVOID) {
        WNDCLASSEXW window_class   = {};
        window_class.cbSize        = sizeof(window_class);
        window_class.lpfnWndProc   = &wnd_proc;
        window_class.hInstance     = GetModuleHandleW(NULL);
        window_class.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
        window_class.hCursor       = LoadCursorA(NULL, IDC_ARROW);
        window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        window_class.lpszClassName = L"Anoynmous-Class";
        RegisterClassExW(&window_class);

        hwnd = CreateWindowExW(
            0, // STYLE NOT VISIBLE.
            window_class.lpszClassName,
            L"Anonymous-Service",
            0,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            window_class.hInstance,
            0);

        for (;;) {
            MSG message;
            GetMessageW(&message, 0, 0, 0);
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        ExitProcess(0);
    }

    static DWORD thread_id;
};

DWORD Service::thread_id = {};
HWND Service::hwnd       = {};

LRESULT CALLBACK display_wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {

    LRESULT result = 0;
    DWORD thread   = (DWORD)GetWindowLongPtrW(window, GWLP_USERDATA);

    switch (message) {
        case WM_CLOSE: {
            PostThreadMessageW(thread, message, (WPARAM)window, lparam);
        } break;
        case WM_SIZE:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_DESTROY:
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR: {
            PostThreadMessageW(thread, message, wparam, lparam);
        } break;
        default: {
            result = DefWindowProcW(window, message, wparam, lparam);
        } break;
    }

    return result;
}

Key to_key(WPARAM wparam) {
#define KEYPAD_ENTER (VK_RETURN + KF_EXTENDED)

    switch (wparam) {
        case VK_TAB: return key_tab;
        case VK_LEFT: return key_leftarrow;
        case VK_RIGHT: return key_rightarrow;
        case VK_UP: return key_uparrow;
        case VK_DOWN: return key_downarrow;
        case VK_PRIOR: return key_pageup;
        case VK_NEXT: return key_pagedown;
        case VK_HOME: return key_home;
        case VK_END: return key_end;
        case VK_INSERT: return key_insert;
        case VK_DELETE: return key_delete;
        case VK_BACK: return key_backspace;
        case VK_SPACE: return key_space;
        case VK_RETURN: return key_enter;
        case VK_ESCAPE: return key_escape;
        case VK_OEM_7: return key_apostrophe;
        case VK_OEM_COMMA: return key_comma;
        case VK_OEM_MINUS: return key_minus;
        case VK_OEM_PERIOD: return key_period;
        case VK_OEM_2: return key_slash;
        case VK_OEM_1: return key_semicolon;
        case VK_OEM_PLUS: return key_equal;
        case VK_OEM_4: return key_leftbracket;
        case VK_OEM_5: return key_backslash;
        case VK_OEM_6: return key_rightbracket;
        case VK_OEM_3: return key_graveaccent;
        case VK_CAPITAL: return key_capslock;
        case VK_SCROLL: return key_scrolllock;
        case VK_NUMLOCK: return key_numlock;
        case VK_SNAPSHOT: return key_printscreen;
        case VK_PAUSE: return key_pause;
        case VK_NUMPAD0: return key_keypad0;
        case VK_NUMPAD1: return key_keypad1;
        case VK_NUMPAD2: return key_keypad2;
        case VK_NUMPAD3: return key_keypad3;
        case VK_NUMPAD4: return key_keypad4;
        case VK_NUMPAD5: return key_keypad5;
        case VK_NUMPAD6: return key_keypad6;
        case VK_NUMPAD7: return key_keypad7;
        case VK_NUMPAD8: return key_keypad8;
        case VK_NUMPAD9: return key_keypad9;
        case VK_DECIMAL: return key_keypaddecimal;
        case VK_DIVIDE: return key_keypaddivide;
        case VK_MULTIPLY: return key_keypadmultiply;
        case VK_SUBTRACT: return key_keypadsubtract;
        case VK_ADD: return key_keypadadd;
        case KEYPAD_ENTER: return key_keypadenter;
        case VK_LSHIFT: return key_leftshift;
        case VK_LCONTROL: return key_leftctrl;
        case VK_LMENU: return key_leftalt;
        case VK_LWIN: return key_leftsuper;
        case VK_RSHIFT: return key_rightshift;
        case VK_RCONTROL: return key_rightctrl;
        case VK_RMENU: return key_rightalt;
        case VK_RWIN: return key_rightsuper;
        case VK_APPS: return key_menu;
        case '0': return key_0;
        case '1': return key_1;
        case '2': return key_2;
        case '3': return key_3;
        case '4': return key_4;
        case '5': return key_5;
        case '6': return key_6;
        case '7': return key_7;
        case '8': return key_8;
        case '9': return key_9;
        case 'A': return key_a;
        case 'B': return key_b;
        case 'C': return key_c;
        case 'D': return key_d;
        case 'E': return key_e;
        case 'F': return key_f;
        case 'G': return key_g;
        case 'H': return key_h;
        case 'I': return key_i;
        case 'J': return key_j;
        case 'K': return key_k;
        case 'L': return key_l;
        case 'M': return key_m;
        case 'N': return key_n;
        case 'O': return key_o;
        case 'P': return key_p;
        case 'Q': return key_q;
        case 'R': return key_r;
        case 'S': return key_s;
        case 'T': return key_t;
        case 'U': return key_u;
        case 'V': return key_v;
        case 'W': return key_w;
        case 'X': return key_x;
        case 'Y': return key_y;
        case 'Z': return key_z;
        case VK_F1: return key_f1;
        case VK_F2: return key_f2;
        case VK_F3: return key_f3;
        case VK_F4: return key_f4;
        case VK_F5: return key_f5;
        case VK_F6: return key_f6;
        case VK_F7: return key_f7;
        case VK_F8: return key_f8;
        case VK_F9: return key_f9;
        case VK_F10: return key_f10;
        case VK_F11: return key_f11;
        case VK_F12: return key_f12;
        default: return key_none;
    }
#undef KEYPAD_ENTER
}

void set_input(UINT message, WPARAM wparam) {
    bool is_key_down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
    Key key          = to_key(wparam);
    Core::io.set_key_state(key, is_key_down);
}

} // namespace

void execute() {
    Service::initialize();

    init_vulkan_resources();
    defer { free_vulkan_resources(); };

    struct Window {
        HWND handle;
        Swapchain swapchain;
        Draw_Data* draw_data;
    } windows[100];
    size_t num_windows = 0;

    WNDCLASSEXW window_class   = {};
    window_class.cbSize        = sizeof(window_class);
    window_class.lpfnWndProc   = &display_wnd_proc;
    window_class.hInstance     = GetModuleHandleW(NULL);
    window_class.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
    window_class.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = L"Anoynmous_Main_Class";
    RegisterClassExW(&window_class);

    Window_Param p = {};
    p.dwExStyle    = 0;
    p.lpClassName  = window_class.lpszClassName;
    p.lpWindowName = L"Tyrant"; // Window name
    p.dwStyle      = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    p.x            = CW_USEDEFAULT;
    p.y            = CW_USEDEFAULT;
    p.nWidth       = CW_USEDEFAULT;
    p.nHeight      = CW_USEDEFAULT;
    p.hInstance    = window_class.hInstance;

    create_shaders_and_pipeline();
    defer { free_shaders_and_pipeline(); };

    auto create_window = [&]() {
        HWND handle =
            (HWND)SendMessageW(Service::hwnd, Window_Messages::create_window, (WPARAM)&p, GetCurrentThreadId());
        auto swapchain         = create_swapchain_from_win32(window_class.hInstance, handle);
        auto draw_data         = create_draw_data();
        windows[num_windows++] = { handle, swapchain, draw_data };
        assert_format(swapchain.format.format);
    };

    auto destroy_window = [&](HWND hwnd) {
        for (size_t i = 0; i < num_windows; ++i) {
            if (hwnd == windows[i].handle) {
                free_swapchain(windows[i].swapchain);
                free_draw_data(windows[i].draw_data);
                SendMessageW(Service::hwnd, Window_Messages::destroy_window, (WPARAM)hwnd, 0);
                windows[i] = windows[--num_windows];
                break;
            }
        }
    };

    defer {
        for (size_t i = 0; i < num_windows; ++i) {
            free_swapchain(windows[i].swapchain);
            free_draw_data(windows[i].draw_data);
        }
    };

    create_window();

    for (; num_windows != 0;) {
        MSG message;
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
            switch (message.message) {
                case WM_SYSKEYUP:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_KEYDOWN: {
                    set_input(message.message, message.wParam);
                    //bool is_key_down = (message.message == WM_KEYDOWN || message.message == WM_SYSKEYDOWN);
                    //if (message.wParam == VK_SHIFT)
                    //     log_info("in wm_keydown shift has been {}", is_key_down ? "pressed" : "released");
                    //if (is_key_down && message.wParam == 'C') create_window();
                    //if (is_key_down && message.wParam == VK_ESCAPE) destroy_window(GetForegroundWindow());
                } break;
                case WM_CLOSE: {
                    destroy_window((HWND)message.wParam);
                } break;
            }
        }

        if (Core::io.key_states[key_leftshift].key_down) {
            log_info("shift has been clicked");
        }
        if (Core::io.key_states[key_escape].key_down) {
            destroy_window(GetForegroundWindow());
        }

        if (Core::io.key_states[key_c].key_down) {
            create_window();
        }

        for (size_t i = 0; i < num_windows; ++i) {
            auto& window = windows[i];
            RECT client;
            GetClientRect(window.handle, &client);
            if (!IsIconic(window.handle) && (client.bottom > client.top && client.right > client.left)) {
                UINT width  = client.right - client.left;
                UINT height = client.bottom - client.top;

                if ((window.swapchain.out_of_date || window.swapchain.width != width ||
                     window.swapchain.height != height)) {
                    resize_swapchain(window.swapchain, width, height);
                }

                draw(window.swapchain, window.draw_data);
                present_swapchain(window.swapchain);
            }
        }

        Core::update();
    }
}

#endif // _WIN32
