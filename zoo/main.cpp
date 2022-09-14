
#include "main/EntryPoint.hpp"

auto main(int argc, char* argv[]) -> int { // NOLINT
    zoo::application::ExitStatus status =
        zoo::main(zoo::application::Settings{argc, argv});
    return status == zoo::application::ExitStatus::ok ? 0 : -1;
}
