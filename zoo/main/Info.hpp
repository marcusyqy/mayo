#pragma once
#include "core/Defines.hpp"
#include <cstdint>
#include <string>

namespace zoo::application {

struct Settings {
    Settings(int argc, char** argv) noexcept;
};

using Version = core::Version;

struct Info {
    Version version_;
    std::string name_;
};

enum ExitStatus { ok, err };

} // namespace zoo::application
