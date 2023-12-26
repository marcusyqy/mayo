#include "ccb/ccb.hpp"
#include <iostream>


int main(int argc, char** argv) {
    using namespace ccb;

    Project project = {};
    project.files = {
        "tyrant/main.cpp"
    };

    project.cpp_version = Cpp_Version::_20;
}
