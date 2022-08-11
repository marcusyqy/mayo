#include <iostream>
#include "EntryPoint.hpp"

namespace zoo {

Application::ExitStatus Main(Application::Settings args) noexcept {
    std::cout << "Hello World from zoo::Main!" << std::endl;
    return Application::ExitStatus::Ok;
}

} // namespace zoo
