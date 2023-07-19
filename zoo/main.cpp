
#include "main/EntryPoint.hpp"

int main(int argc, char* argv[]) { // NOLINT
// TODO: check if this is only for windows
#if defined(WIN32)
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

    zoo::application::ExitStatus status = zoo::main(zoo::application::Settings{ argc, argv });
    return status == zoo::application::ExitStatus::ok ? 0 : -1;
}
