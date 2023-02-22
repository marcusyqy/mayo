#pragma once
#include "core/fwd.hpp"
#include <cstdint>
#include <string>

namespace zoo::application {

struct Settings {
    Settings(int num_args, char** args) noexcept;
};

using Version = core::Version;

struct Info {
    Version version;
    std::string name;
};

enum ExitStatus { ok, err };

} // namespace zoo::application
