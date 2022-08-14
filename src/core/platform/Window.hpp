#pragma once
#include <functional>
#include <string_view>

#include "Input.hpp"

namespace zoo {

class WindowFactory {
public:
    using ErrorCallback = std::function<void(std::string_view description)>;
    void SetErrorCallback(ErrorCallback&& description);
private:
};

struct WindowTraits {};

class Window {
public:
    using InputCallback = std::function<void(input::KeyCode)>;
    void SetKeyCallback(InputCallback&& callback);

private:
    WindowTraits windowTraits_;
};
} // namespace zoo