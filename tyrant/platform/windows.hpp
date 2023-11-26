#pragma once
#include <windows.h>
#include <stdio.h>
#include "logger.hpp"

#pragma comment (lib, "kernel32")
#pragma comment (lib, "user32")
#pragma comment (lib, "gdi32")

/* Copied from DTC by cmuratori
   "https://github.com/cmuratori/dtc"
*/
struct Actual_Window_Param {
    DWORD     dwExStyle;
    LPCWSTR   lpClassName;
    LPCWSTR   lpWindowName;
    DWORD     dwStyle;
    int       x;
    int       y;
    int       nWidth;
    int       nHeight;
    HWND      hWndParent;
    HMENU     hMenu;
    HINSTANCE hInstance;
    LPVOID    lpParam;
};

#define CREATE_DANGEROUS_WINDOW (WM_USER + 0x1337)
#define DESTROY_DANGEROUS_WINDOW (WM_USER + 0x1338)

static DWORD main_thread_id;

static LRESULT CALLBACK service_wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch(message) {
        case CREATE_DANGEROUS_WINDOW: {
            auto* p = (Actual_Window_Param*)wparam;
            result = (LRESULT)CreateWindowExW(
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
            break;
        }
        case DESTROY_DANGEROUS_WINDOW: {
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

static LRESULT CALLBACK display_wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    /* NOTE(casey): This is an example of an actual window procedure. It doesn't do anything
       but forward things to the main thread, because again, all window messages now occur
       on the message thread, and presumably we would rather handle everything there.  You
       don't _have_ to do that - you could choose to handle some of the messages here.
       But if you did, you would have to actually think about whether there are race conditions
       with your main thread and all that.  So just PostThreadMessageW()'ing everything gets
       you out of having to think about it.
    */

    LRESULT result = 0;

    switch (message) {
        // NOTE(casey): Mildly annoying, if you want to specify a window, you have
        // to snuggle the params yourself, because Windows doesn't let you forward
        // a god damn window message even though the program IS CALLED WINDOWS. It's
        // in the name! Let me pass it!
        case WM_CLOSE: {
            PostThreadMessageW(main_thread_id, message, (WPARAM)window, lparam);
        } break;

        // NOTE(casey): Anything you want the application to handle, forward to the main thread
        // here.
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_DESTROY:
        case WM_CHAR: {
            PostThreadMessageW(main_thread_id, message, wparam, lparam);
        } break;

        default: {
            result = DefWindowProcW(window, message, wparam, lparam);
        } break;
    }

    return result;
}

static DWORD WINAPI main_thread(LPVOID param) {
    /* NOTE(Casey): This is your app code. Basically you just do everything the same,
       but instead of calling CreateWindow/DestroyWindow, you use SendMessage to
       do it on the other thread, using the CREATE_DANGEROUS_WINDOW and DESTROY_DANGEROUS_WINDOW
       user messages.  Otherwise, everything proceeds as normal.
    */
    HWND service_window = (HWND)param;

    WNDCLASSEXW window_class = {};
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = &display_wnd_proc;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = L"TyrantMain Class";
    RegisterClassExW(&window_class);

    Actual_Window_Param p = {};
    p.dwExStyle = 0;;
    p.lpClassName = window_class.lpszClassName;
    p.lpWindowName = L"Tyrant";
    p.dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    p.x = CW_USEDEFAULT;
    p.y = CW_USEDEFAULT;
    p.nWidth = CW_USEDEFAULT;
    p.nHeight = CW_USEDEFAULT;
    p.hInstance = window_class.hInstance;
    [[maybe_unused]] HWND handle = (HWND)SendMessageW(service_window, CREATE_DANGEROUS_WINDOW, (WPARAM)&p, 0);

    int x = 0;
    for(;;) {
        MSG message;
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
            switch(message.message) {
                case WM_CHAR: {
                    SendMessageW(service_window, CREATE_DANGEROUS_WINDOW, (WPARAM)&p, 0);
                } break;

                case WM_CLOSE: {
                    SendMessageW(service_window, DESTROY_DANGEROUS_WINDOW, message.wParam, 0);
                } break;
            }
        }

        int mid_point = (x++%(64*1024))/64;
        int window_count = 0;
        for(HWND window = FindWindowExW(0, 0, window_class.lpszClassName, 0);
            window;
            window = FindWindowExW(0, window, window_class.lpszClassName, 0)) {
            // Change this to game loop ? Do something here.
            RECT client;
            GetClientRect(window, &client);
            HDC device_context = GetDC(window);

            PatBlt(device_context, 0, 0, mid_point, client.bottom, BLACKNESS);
            if(client.right > mid_point) {
                PatBlt(device_context, mid_point, 0, client.right - mid_point, client.bottom, WHITENESS);
            }
            ReleaseDC(window, device_context);

            ++window_count;
        }

        if(window_count == 0) {
            break;
        }
    }

    ExitProcess(0);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    log_info("starting application");

    WNDCLASSEXW window_class = {};
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = &service_wnd_proc;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = L"Tyrant-Class";
    RegisterClassExW(&window_class);

    HWND service_window =
        CreateWindowExW(
            0, window_class.lpszClassName, L"Tyrant-Service", 0,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, window_class.hInstance, 0);

    // TODO:
    CreateThread(0, 0, main_thread, service_window, 0, &main_thread_id);

    for(;;) {
        MSG message;
        GetMessageW(&message, 0, 0, 0);
        TranslateMessage(&message);

        if((message.message == WM_CHAR) ||
           (message.message == WM_KEYDOWN) ||
           (message.message == WM_QUIT) ||
           (message.message == WM_SIZE)) {
            PostThreadMessageW(main_thread_id, message.message, message.wParam, message.lParam);
        } else {
            DispatchMessageW(&message);
        }
    }

    return 0;
}

#ifndef RELEASE_BUILD
// i want a console. In non-release build
int main(int argc, char** argv) {
    logger_init();
    log_trace("This is a debug build");
    WinMain(0, 0, 0, 0);
}

#endif // RELEASE_BUILD

// for msvc:
// run command: "cl tyrant/main.cpp /std:c++17 /Fe:build/main.exe /Fo:build/"
