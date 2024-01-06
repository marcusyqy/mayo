#ifdef WIN32

#define NO_ENTRY_POINT
#include "windows.hpp"
#include "../render.hpp"

#include "basic.hpp"
#include "logger.hpp"
#include <algorithm>
#include <windows.h>

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

namespace {

DWORD service_thread_id;
HWND service_window;

struct Actual_Window_Param {
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
        defer_wm_close = WM_USER + 0x1339,
    };
};

LRESULT CALLBACK service_wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch (message) {
        case Window_Messages::create_window: {
            auto* p      = (Actual_Window_Param*)wparam;
            DWORD thread = (DWORD)lparam;
            auto hwnd    = CreateWindowExW(
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

            SetWindowLongPtrW(hwnd, GWLP_USERDATA, thread);
            result = (LRESULT)hwnd;
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

LRESULT CALLBACK display_wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {

    LRESULT result = 0;
    DWORD thread   = (DWORD)GetWindowLongPtrW(window, GWLP_USERDATA);

    switch (message) {
        // NOTE(casey): Mildly annoying, if you want to specify a window, you have
        // to snuggle the params yourself, because Windows doesn't let you forward
        // a god damn window message even though the program IS CALLED WINDOWS. It's
        // in the name! Let me pass it!    
         case WM_SIZE:
         case WM_CLOSE: {
            // return FALSE;
            // PostThreadMessageW(GetWindowThreadProcessId(window, 0), message, (WPARAM)window, lparam);
            //  PostMessageW(window, WM_CLOSE, (WPARAM)window, lparam);
            DWORD thread = (DWORD)GetWindowLongPtrW(window, GWLP_USERDATA);
            PostThreadMessageW(thread, message, (WPARAM)window, lparam);
            // SendMessageW(window, WM_CLOSE, (WPARAM)window, lparam);
            // PostThreadMessageW(event_thread_id, message, (WPARAM)window, lparam);
        } break;

        // NOTE(casey): Anything you want the application to handle, forward to the main thread
        // here.
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_DESTROY:
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR: {
            // PostThreadMessageW(event_thread_id, message, wparam, lparam);
            DWORD thread = (DWORD)GetWindowLongPtrW(window, GWLP_USERDATA);
            PostThreadMessageW(thread, message, wparam, lparam);
            // result = DefWindowProcW(window, message, wparam, lparam);
        } break;
        default: {
            result = DefWindowProcW(window, message, wparam, lparam);
            // PostThreadMessageW(service_thread_id, message, wparam, lparam);
            //  PostThreadMessageW(event_thread_id, message, (WPARAM)window, lparam);
        } break;
    }

    return result;
}

DWORD WINAPI service_thread(LPVOID param) {
    // DWORD main_thread_id = (DWORD)param;

    WNDCLASSEXW window_class   = {};
    window_class.cbSize        = sizeof(window_class);
    window_class.lpfnWndProc   = &service_wnd_proc;
    window_class.hInstance     = GetModuleHandleW(NULL);
    window_class.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
    window_class.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = L"Tyrant-Class";
    RegisterClassExW(&window_class);

    service_window = CreateWindowExW(
        0, // STYLE NOT VISIBLE.
        window_class.lpszClassName,
        L"Tyrant-Service",
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
        // if ((message.message == WM_CHAR) || (message.message == WM_KEYDOWN) || (message.message == WM_QUIT) ||
        //     (message.message == WM_SIZE)) {
        //     PostThreadMessageW(main_thread_id, message.message, message.wParam, message.lParam);
        // } else {
        //     DispatchMessageW(&message);
        // }
    }

    ExitProcess(0);
}

void initialize_and_create_empty_window() {
    // create event thread
    CreateThread(0, 0, service_thread, (LPVOID)GetCurrentThreadId(), 0, &service_thread_id);
}

} // namespace

void execute() {
    initialize_and_create_empty_window();

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
    window_class.lpszClassName = L"Tyrant_Main_Class";
    RegisterClassExW(&window_class);

    Actual_Window_Param p = {};
    p.dwExStyle           = 0;
    p.lpClassName         = window_class.lpszClassName;
    p.lpWindowName        = L"Tyrant"; // Window name
    p.dwStyle             = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    p.x                   = CW_USEDEFAULT;
    p.y                   = CW_USEDEFAULT;
    p.nWidth              = CW_USEDEFAULT;
    p.nHeight             = CW_USEDEFAULT;
    p.hInstance           = window_class.hInstance;

    create_shaders_and_pipeline();
    defer { free_shaders_and_pipeline(); };

    auto create_window = [&]() {
        HWND handle =
            (HWND)SendMessageW(service_window, Window_Messages::create_window, (WPARAM)&p, GetCurrentThreadId());
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
                SendMessageW(service_window, Window_Messages::destroy_window, (WPARAM)hwnd, 0);
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
                // this shouldn't be the way we handle inputs.
                case WM_CHAR: {
                    //  when we support multiple swap chains
                    // SendMessageW(service_window, Window_Messages::create_window, (WPARAM)&p, 0);
                    // if (message.wParam == VK_ESCAPE)
                    //     SendMessageW(service_window, Window_Messages::destroy_window,
                    //     (WPARAM)GetForegroundWindow(), 0);
                    // if (std::tolower(message.wParam) == 'c')
                    //     SendMessageW(service_window, Window_Messages::create_window, (WPARAM)&p, 0);
                    // if (message.wParam == VK_LSHIFT) log_info("shift has been pressed");
                } break;
                case WM_KEYDOWN:
                    if (message.wParam == VK_SHIFT) log_info("in wm_keydown shift has been pressed");
                    if (message.wParam == 'C') create_window();
                    //                        (HWND) SendMessageW(service_window, Window_Messages::create_window,
                    //                        (WPARAM)&p, 0);
                    if (message.wParam == VK_ESCAPE) destroy_window(GetForegroundWindow());
                    //                       SendMessageW(service_window, Window_Messages::destroy_window,
                    //                       (WPARAM)GetForegroundWindow(), 0);
                    // etc.
                    break;
                case WM_KEYUP:
                    if (message.wParam == VK_SHIFT) log_info("in wm_keyup shift has been released");
                    // etc.
                    break;
                case WM_CLOSE: {
                    destroy_window((HWND)message.wParam);
                    // SendMessageW(service_window, Window_Messages::destroy_window, message.wParam, 0);
                } break;
            }
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
    }
}

#endif // WIN32
