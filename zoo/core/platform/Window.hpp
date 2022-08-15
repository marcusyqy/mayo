#pragma once
#include <functional>
#include <string_view>

#include "Input.hpp"
#include "main/Application.hpp"

namespace zoo {

class WindowFactory {
public:
    using ErrorCallback = std::function<void(std::string_view description)>;

    WindowFactory(const Application::Context& context, ErrorCallback description) noexcept;
    ~WindowFactory() noexcept;


private:
    ErrorCallback callback_;
};

struct WindowTraits {
    bool fullScreen_;
};

class Window {
public:
    using InputCallback = std::function<void(input::KeyCode)>;
    Window(InputCallback callback) noexcept;

private:
    WindowTraits windowTraits_;
    InputCallback callback_;
};

} // namespace zoo