
#include "main/EntryPoint.hpp"

int main(int argc, char* argv[]) { // NOLINT
    zoo::Application::ExitStatus status = zoo::Main(zoo::Application::Settings{argc, argv});
    return status == zoo::Application::ExitStatus::Ok ? 0 : -1;
}
