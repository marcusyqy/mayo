
#include "adapters/imgui/Layer.hpp"
#include "core/Utils.hpp"

int main(int argc, char* argv[]) { // NOLINT
    static_cast<void>(argc);
    static_cast<void>(argv);

    using namespace zoo;
    core::check_memory();
    adapters::imgui::test();
    return 0;
}
