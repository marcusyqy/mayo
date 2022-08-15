#pragma once
#include <functional>
#include <string_view>

#include "Input.hpp"
#include "main/Application.hpp"

namespace zoo {

class WindowFactory {
public:
    using ErrorCallback = std::function<void(std::string_view description)>;

    WindowFactory(const Application::Context& context) noexcept;
    ~WindowFactory() noexcept;
    void SetErrorCallback(ErrorCallback&& description) noexcept;

private:

};

struct WindowTraits {
    bool fullScreen;
};

class Window {
public:
    using InputCallback = std::function<void(input::KeyCode)>;
    void SetKeyCallback(InputCallback&& callback);

private:
    WindowTraits windowTraits_;
};

} // namespace zoo