#pragma once

#include "Info.hpp"

namespace zoo::main {

class Application {
public:
    Application(const application::Info& info) noexcept;

private:
    application::Info info_;
};

} // namespace zoo::main
