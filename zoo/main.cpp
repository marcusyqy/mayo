
#include "main/EntryPoint.hpp"

int main(int argc, char* argv[]) { // NOLINT
    zoo::Application::ExitStatus status =
        zoo::main(zoo::Application::Settings{argc, argv});
    return status == zoo::Application::ExitStatus::ok ? 0 : -1;
}
