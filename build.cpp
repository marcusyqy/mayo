#include "zoo/build.hpp"
#include "ccb/ccb.hpp"
#include <iostream>

int main(int argc, char** argv) {
    using namespace ccb;

    Workspace ws;
    ws.name         = "zoo";
    ws.architecture = Architecture::x86_64;

    static_cast<void>(argc);
    static_cast<void>(argv);
    std::cout << "hello" << std::endl;
}
