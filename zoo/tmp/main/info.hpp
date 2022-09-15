#pragma once
#include "core/defines.hpp"
#include <cstdint>
#include <string>

namespace zoo::detail::application {

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
