#include "zoo/build.hpp"
#include "ccb/ccb.hpp"
#include <iostream>

int main(int argc, char** argv) {
    using namespace ccb;

    Default_Workspace ws{};
    ws.name         = "zoo";
    ws.architecture = Architecture::x86_64;

    ws.configurations = { "Debug", "Release", "Dist" };
    ws.flags = Workspace_Flags::MULTI_PROCESSOR_COMPILE;
    // ws.output_dir =  "

    static_cast<void>(argc);
    static_cast<void>(argv);
    std::cout << "hello" << std::endl;
}
