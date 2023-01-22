
#include "main/entry_point.hpp"

int main(int argc, char* argv[]) { // NOLINT
    // TODO: check if this is only for windows
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    zoo::application::exit_status status =
        zoo::main(zoo::application::settings{argc, argv});
    return status == zoo::application::exit_status::ok ? 0 : -1;
}
