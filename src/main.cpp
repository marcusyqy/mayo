#include "main/entry.hpp"

int main(int argc, char* argv[]) { // NOLINT
    zoo::application::exit_status status = zoo::main(zoo::application::settings{argc, argv});
    return status != zoo::application::exit_status::ok ? -1 : 0;
}
