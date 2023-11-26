#include "ccb/ccb.hpp"
#include <iostream>

namespace ccb {

void build(const Project* project, Source_Location loc) {
    const char* source_path = loc.file_name;
}

}

int main(int argc, char** argv) {
    using namespace ccb;

    Project project = {};
    project.files = {
        "tyrant/main.cpp"
    };

    project.cpp_version = Cpp_Version::_20;

    build(&project);
}
