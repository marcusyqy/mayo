
#include "adapters/imgui/Layer.hpp"
#include "core/Utils.hpp"

int main(int argc, char* argv[]) { // NOLINT
    static_cast<void>(argc);
    static_cast<void>(argv);

    using namespace zoo;

    core::attach_debug();
    adapters::imgui::test();
    return 0;
}
