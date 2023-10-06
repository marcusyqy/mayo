#pragma once
#include <functional>
#include <string_view>

#include "core/fwd.hpp"

#include "input.hpp"

#include "utility/registry.hpp"
#include <memory>
#include <stdx/span.hpp>
#include <vector>

#include "stdx/enum.hpp"

struct GLFWwindow;

namespace zoo {

enum class Window_Event_Type { QUIT, KEY, RESIZE };

struct Window_Event {
    Window_Event_Type type;

    // Input event data
    Key_Code key_code = {};

    // Resize event data
    s32 width  = -1;
    s32 height = -1;
};

struct Quit_Event : Window_Event {
    Quit_Event() : Window_Event{ Window_Event_Type::QUIT } {}
};

struct Input_Event : Window_Event {
    Input_Event(Key_Code kc) : Window_Event{ Window_Event_Type::KEY, kc } {}
};

struct Resize_Event : Window_Event {
    Resize_Event(s32 width, s32 height) : Window_Event{ Window_Event_Type::RESIZE, {}, width, height } {}
};

// clang-format off
enum class Window_Traits {
    NONE = 0,
    RESIZABLE = 1 << 0,
    VISIBLE = 1 << 1,
    FOCUS_ON_SHOW = 1 << 2,
    _stdx_enum_as_bitmask
};
// clang-format on

class Window {
public:
    explicit Window(s32 width, s32 height, std::string_view name) noexcept;
    ~Window() noexcept;

    Window(Window&& other) noexcept                 = delete;
    Window(const Window& other) noexcept            = delete;
    Window& operator=(Window&& other) noexcept      = delete;
    Window& operator=(const Window& other) noexcept = delete;

    void swap_buffers() noexcept;
    bool valid() const noexcept { return impl_ != nullptr; }
    operator bool() const noexcept { return valid(); }

    // We need this now for ImGui. (remove after swapping to some different API for windows).
    GLFWwindow* impl() const noexcept { return impl_; }

    stdx::span<const Window_Event> events_this_frame() const noexcept;

    static void poll_events() noexcept;

    s32 width() const noexcept { return width_; }
    s32 height() const noexcept { return height_; }

    std::string_view get_clipboard_text() const;
    void set_clipboard_text(std::string_view text);

private:
    void install_callbacks();
    friend struct Glfw_Callbacks;

private:
    s32 width_;
    s32 height_;
    std::string name_;

    GLFWwindow* impl_;
    //
    std::vector<Window_Event> events_;
};

} // namespace zoo
