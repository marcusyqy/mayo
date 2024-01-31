#include "imgui_adapter.hpp"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>

// for win32
#define NO_ENTRY_POINT
#include "../os/windows.hpp"

namespace glue::imgui {

namespace {
namespace win32_callback {

void create_window(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = IM_NEW(ImGui_ImplWin32_ViewportData)();
    viewport->PlatformUserData       = vd;

    // Select style and parent window
    ImGui_ImplWin32_GetWin32StyleFromViewportFlags(viewport->Flags, &vd->DwStyle, &vd->DwExStyle);
    vd->HwndParent = ImGui_ImplWin32_GetHwndFromViewportID(viewport->ParentViewportId);

    // Create window
    RECT rect = { (LONG)viewport->Pos.x,
                  (LONG)viewport->Pos.y,
                  (LONG)(viewport->Pos.x + viewport->Size.x),
                  (LONG)(viewport->Pos.y + viewport->Size.y) };
    ::AdjustWindowRectEx(&rect, vd->DwStyle, FALSE, vd->DwExStyle);
    vd->Hwnd = ::CreateWindowEx(
        vd->DwExStyle,
        _T("ImGui Platform"),
        _T("Untitled"),
        vd->DwStyle, // Style, class name, window name
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top, // Window area
        vd->HwndParent,
        nullptr,
        ::GetModuleHandle(nullptr),
        nullptr); // Owner window, Menu, Instance, Param
    vd->HwndOwned                   = true;
    viewport->PlatformRequestResize = false;
    viewport->PlatformHandle = viewport->PlatformHandleRaw = vd->Hwnd;
}

void destroy_window(ImGuiViewport* viewport) {
    ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData();
    if (ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData) {
        if (::GetCapture() == vd->Hwnd) {
            // Transfer capture so if we started dragging from a window that later disappears, we'll still receive the
            // MOUSEUP event.
            ::ReleaseCapture();
            ::SetCapture(bd->hWnd);
        }
        if (vd->Hwnd && vd->HwndOwned) ::DestroyWindow(vd->Hwnd);
        vd->Hwnd = nullptr;
        IM_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

void ImGui_ImplWin32_ShowWindow(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing) ::ShowWindow(vd->Hwnd, SW_SHOWNA);
    else
        ::ShowWindow(vd->Hwnd, SW_SHOW);
}

void ImGui_ImplWin32_UpdateWindow(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);

    // Update Win32 parent if it changed _after_ creation
    // Unlike style settings derived from configuration flags, this is more likely to change for advanced apps that are
    // manipulating ParentViewportID manually.
    HWND new_parent = ImGui_ImplWin32_GetHwndFromViewportID(viewport->ParentViewportId);
    if (new_parent != vd->HwndParent) {
        // Win32 windows can either have a "Parent" (for WS_CHILD window) or an "Owner" (which among other thing keeps
        // window above its owner). Our Dear Imgui-side concept of parenting only mostly care about what Win32 call
        // "Owner". The parent parameter of CreateWindowEx() sets up Parent OR Owner depending on WS_CHILD flag. In our
        // case an Owner as we never use WS_CHILD. Calling ::SetParent() here would be incorrect: it will create a full
        // child relation, alter coordinate system and clipping. Calling ::SetWindowLongPtr() with GWLP_HWNDPARENT seems
        // correct although poorly documented. https://devblogs.microsoft.com/oldnewthing/20100315-00/?p=14613
        vd->HwndParent = new_parent;
        ::SetWindowLongPtr(vd->Hwnd, GWLP_HWNDPARENT, (LONG_PTR)vd->HwndParent);
    }

    // (Optional) Update Win32 style if it changed _after_ creation.
    // Generally they won't change unless configuration flags are changed, but advanced uses (such as manually rewriting
    // viewport flags) make this useful.
    DWORD new_style;
    DWORD new_ex_style;
    ImGui_ImplWin32_GetWin32StyleFromViewportFlags(viewport->Flags, &new_style, &new_ex_style);

    // Only reapply the flags that have been changed from our point of view (as other flags are being modified by
    // Windows)
    if (vd->DwStyle != new_style || vd->DwExStyle != new_ex_style) {
        // (Optional) Update TopMost state if it changed _after_ creation
        bool top_most_changed = (vd->DwExStyle & WS_EX_TOPMOST) != (new_ex_style & WS_EX_TOPMOST);
        HWND insert_after =
            top_most_changed ? ((viewport->Flags & ImGuiViewportFlags_TopMost) ? HWND_TOPMOST : HWND_NOTOPMOST) : 0;
        UINT swp_flag = top_most_changed ? 0 : SWP_NOZORDER;

        // Apply flags and position (since it is affected by flags)
        vd->DwStyle   = new_style;
        vd->DwExStyle = new_ex_style;
        ::SetWindowLong(vd->Hwnd, GWL_STYLE, vd->DwStyle);
        ::SetWindowLong(vd->Hwnd, GWL_EXSTYLE, vd->DwExStyle);
        RECT rect = { (LONG)viewport->Pos.x,
                      (LONG)viewport->Pos.y,
                      (LONG)(viewport->Pos.x + viewport->Size.x),
                      (LONG)(viewport->Pos.y + viewport->Size.y) };
        ::AdjustWindowRectEx(&rect, vd->DwStyle, FALSE, vd->DwExStyle); // Client to Screen
        ::SetWindowPos(
            vd->Hwnd,
            insert_after,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            swp_flag | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        ::ShowWindow(vd->Hwnd, SW_SHOWNA); // This is necessary when we alter the style
        viewport->PlatformRequestMove = viewport->PlatformRequestResize = true;
    }
}

ImVec2 ImGui_ImplWin32_GetWindowPos(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    POINT pos = { 0, 0 };
    ::ClientToScreen(vd->Hwnd, &pos);
    return ImVec2((float)pos.x, (float)pos.y);
}

void ImGui_ImplWin32_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    RECT rect = { (LONG)pos.x, (LONG)pos.y, (LONG)pos.x, (LONG)pos.y };
    ::AdjustWindowRectEx(&rect, vd->DwStyle, FALSE, vd->DwExStyle);
    ::SetWindowPos(vd->Hwnd, nullptr, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

ImVec2 ImGui_ImplWin32_GetWindowSize(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    RECT rect;
    ::GetClientRect(vd->Hwnd, &rect);
    return ImVec2(float(rect.right - rect.left), float(rect.bottom - rect.top));
}

void ImGui_ImplWin32_SetWindowSize(ImGuiViewport* viewport, ImVec2 size) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    RECT rect = { 0, 0, (LONG)size.x, (LONG)size.y };
    ::AdjustWindowRectEx(&rect, vd->DwStyle, FALSE, vd->DwExStyle); // Client to Screen
    ::SetWindowPos(
        vd->Hwnd,
        nullptr,
        0,
        0,
        rect.right - rect.left,
        rect.bottom - rect.top,
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

void ImGui_ImplWin32_SetWindowFocus(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    ::BringWindowToTop(vd->Hwnd);
    ::SetForegroundWindow(vd->Hwnd);
    ::SetFocus(vd->Hwnd);
}

bool ImGui_ImplWin32_GetWindowFocus(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    return ::GetForegroundWindow() == vd->Hwnd;
}

bool ImGui_ImplWin32_GetWindowMinimized(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    return ::IsIconic(vd->Hwnd) != 0;
}

void ImGui_ImplWin32_SetWindowTitle(ImGuiViewport* viewport, const char* title) {
    // ::SetWindowTextA() doesn't properly handle UTF-8 so we explicitely convert our string.
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    int n = ::MultiByteToWideChar(CP_UTF8, 0, title, -1, nullptr, 0);
    ImVector<wchar_t> title_w;
    title_w.resize(n);
    ::MultiByteToWideChar(CP_UTF8, 0, title, -1, title_w.Data, n);
    ::SetWindowTextW(vd->Hwnd, title_w.Data);
}

void ImGui_ImplWin32_SetWindowAlpha(ImGuiViewport* viewport, float alpha) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    IM_ASSERT(alpha >= 0.0f && alpha <= 1.0f);
    if (alpha < 1.0f) {
        DWORD style = ::GetWindowLongW(vd->Hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
        ::SetWindowLongW(vd->Hwnd, GWL_EXSTYLE, style);
        ::SetLayeredWindowAttributes(vd->Hwnd, 0, (BYTE)(255 * alpha), LWA_ALPHA);
    } else {
        DWORD style = ::GetWindowLongW(vd->Hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
        ::SetWindowLongW(vd->Hwnd, GWL_EXSTYLE, style);
    }
}

float ImGui_ImplWin32_GetWindowDpiScale(ImGuiViewport* viewport) {
    ImGui_ImplWin32_ViewportData* vd = (ImGui_ImplWin32_ViewportData*)viewport->PlatformUserData;
    IM_ASSERT(vd->Hwnd != 0);
    return ImGui_ImplWin32_GetDpiScaleForHwnd(vd->Hwnd);
}

// FIXME-DPI: Testing DPI related ideas
void ImGui_ImplWin32_OnChangedViewport(ImGuiViewport* viewport) {
    (void)viewport;
#if 0
    ImGuiStyle default_style;
    //default_style.WindowPadding = ImVec2(0, 0);
    //default_style.WindowBorderSize = 0.0f;
    //default_style.ItemSpacing.y = 3.0f;
    //default_style.FramePadding = ImVec2(0, 0);
    default_style.ScaleAllSizes(viewport->DpiScale);
    ImGuiStyle& style = ImGui::GetStyle();
    style = default_style;
#endif
}

BOOL CALLBACK update_monitor_callback(HMONITOR monitor, HDC, LPRECT, LPARAM) {
    MONITORINFO info = {};
    info.cbSize      = sizeof(MONITORINFO);
    if (!::GetMonitorInfo(monitor, &info)) return TRUE;
    ImGuiPlatformMonitor imgui_monitor;
    imgui_monitor.MainPos  = ImVec2((float)info.rcMonitor.left, (float)info.rcMonitor.top);
    imgui_monitor.MainSize = ImVec2(
        (float)(info.rcMonitor.right - info.rcMonitor.left),
        (float)(info.rcMonitor.bottom - info.rcMonitor.top));
    imgui_monitor.WorkPos = ImVec2((float)info.rcWork.left, (float)info.rcWork.top);
    imgui_monitor.WorkSize =
        ImVec2((float)(info.rcWork.right - info.rcWork.left), (float)(info.rcWork.bottom - info.rcWork.top));
    imgui_monitor.DpiScale       = ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);
    imgui_monitor.PlatformHandle = (void*)monitor;
    ImGuiPlatformIO& io          = ImGui::GetPlatformIO();
    if (info.dwFlags & MONITORINFOF_PRIMARY) io.Monitors.push_front(imgui_monitor);
    else
        io.Monitors.push_back(imgui_monitor);
    return TRUE;
}

} // namespace win32_callback

} // namespace

bool init(void* hwnd) {
    auto& io = ImGui::GetIO();
    bool ret = ImGui_ImplWin32_Init(hwnd);
    if()
    return ret;
}

} // namespace glue::imgui