
#include "main/EntryPoint.hpp"

int main(int argc, char* argv[]) { // NOLINT
    zoo::application::ExitStatus status =
        zoo::main(zoo::application::Settings{argc, argv});
    return status == zoo::application::ExitStatus::ok ? 0 : -1;
}
