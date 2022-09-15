
#include "main/entry_point.hpp"

auto main(int argc, char* argv[]) -> int { // NOLINT
    zoo::application::exit_status status =
        zoo::main(zoo::application::settings{argc, argv});
    return status == zoo::application::exit_status::ok ? 0 : -1;
}
